#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "asm.h"

extern "C" DWORD _tls_index;

namespace
{
const unsigned char OPCODE_TEMPLATE[] = {
    0xB8,                                           // mov eax, <tls_index>
    0, 0, 0, 0,
    0x65, 0x4C, 0x8B, 0x14, 0x25, 0x58, 0, 0, 0,    // mov r10, qword ptr gs:[88]
    0x4D, 0x8B, 0x14, 0xC2,                         // mov r10, qword ptr [r10+rax*8]
    0x48, 0xB8,                                     // mov rax, <offset>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x49, 0xBB,                                     // mov r11, <arg>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x4E, 0x89, 0x1C, 0x10,                         // mov qword ptr[rax + r10], r11
    0x48, 0xB8,                                     // mov rax, fn_addr
    0, 0, 0, 0, 0, 0, 0, 0,
    0x50,                                           // push rax
    0xC3,                                           // ret
};

const std::size_t TLS_INDEX_OFF = 0x01;
const std::size_t ARG_OFFSET_OFF = 0x14;
const std::size_t ARG_OFF = 0x1E;
const std::size_t FN_ADDR_OFF = 0x2C;

std::size_t get_thread_local_offset(const void* addr)
{
    const char* base = *reinterpret_cast<const char**>(__readgsqword(88) + 4 * _tls_index);
    return reinterpret_cast<const char*>(addr) - base;
}
}

namespace fn2ptr
{
std::size_t get_opcode_size()
{
    return sizeof(OPCODE_TEMPLATE);
}

void write_opcode(char* buffer, const void* fn_addr, void* arg, void** arg_addr)
{
    const auto arg_offset = get_thread_local_offset(arg_addr);
    memcpy(buffer, OPCODE_TEMPLATE, sizeof(OPCODE_TEMPLATE));
    memcpy(buffer + FN_ADDR_OFF, &fn_addr, sizeof(fn_addr));
    memcpy(buffer + ARG_OFFSET_OFF, &arg_offset, sizeof(arg_offset));
    memcpy(buffer + TLS_INDEX_OFF, &_tls_index, sizeof(_tls_index));
    memcpy(buffer + ARG_OFF, &arg, sizeof(arg));
}
}
