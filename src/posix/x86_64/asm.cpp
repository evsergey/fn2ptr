#include "asm.h"

#include <cstring>

namespace
{
const unsigned char OPCODE_TEMPLATE[] = {
    0x51,                                           // push rcx
    0x64, 0x48, 0x8B, 0x0C, 0x25, 0, 0, 0, 0,       // mov rcx, qword ptr fs:[0]
    0x48, 0xB8,                                     // mov rax, <offset>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x48, 0x29, 0xC1,                               // sub rcx, rax
    0x48, 0xB8,                                     // mov rax, <arg>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x48, 0x89, 0x01,                               // mov qword ptr[rcx], rax
    0x59,                                           // pop rcx
    0x48, 0xB8,                                     // mov rax, fn_addr
    0, 0, 0, 0, 0, 0, 0, 0,
    0x50,                                           // push rax
    0xC3,                                           // ret
};

const std::size_t ARG_OFFSET_OFF = 0x0C;
const std::size_t ARG_OFF = 0x19;
const std::size_t FN_ADDR_OFF = 0x27;

std::size_t get_thread_local_offset(const void* addr)
{
    char* base;
    asm(
        "movq %%fs:0, %%rax;"
        "movq %%rax, %0;"
        :"=r"(base)
        :
        : "%rax"
    );
    return base - reinterpret_cast<const char*>(addr);
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
    memcpy(buffer + ARG_OFF, &arg, sizeof(arg));
}
}
