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

const unsigned char INTER_OPCODE_TEMPLATE[] = {
    0x51, 0x52, 0x41, 0x50, 0x41, 0x51,             // push cx, dx, r8, r9
    0x48, 0x83, 0xEC, 0x40,                         // sub rsp, 40h
    0xF3, 0x0F, 0x7F, 0x04, 0x24,                   // movdqu [rsp], xmm0
    0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x10,             // movdqu [rsp+10h], xmm1
    0xF3, 0x0F, 0x7F, 0x54, 0x24, 0x20,             // movdqu [rsp+20h], xmm2
    0xF3, 0x0F, 0x7F, 0x5C, 0x24, 0x30,             // movdqu [rsp+30h], xmm3
    0x4C, 0x8D, 0x44, 0x24, 0x60,                   // lea r8, [rsp+60h]
    0x30, 0xD2,                                     // xor dl, dl
    0x48, 0xB9,                                     // mov rcx, <arg>;  label1
    0, 0, 0, 0, 0, 0, 0, 0,
    0x48, 0x83, 0xEC, 0x20,                         // sub rsp, 32
    0x48, 0x8D, 0x05, 0x0D, 0, 0, 0,                // lea rax, [rip + 0x0D]; ref label2
    0x50,                                           // push rax
    0x48, 0xB8,                                     // mov rax, <inter>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x50,                                           // push rax
    0xC3,                                           // ret
    0x48, 0x83, 0xC4, 0x20,                         // add rsp, 32; label2
    0x48, 0x09, 0xC0,                               // or rax, rax
    0x0F, 0x85, 0x0B, 0, 0, 0,                      // jnz 0x0B
    0xF3, 0x0F, 0x6F, 0x04, 0x24,                   // movdqu xmm0, [rsp]
    0x48, 0x83, 0xC4, 0x10,                         // add rsp, 10h
    0x58,                                           // pop rax
    0xC3,                                           // ret
    0xF3, 0x0F, 0x6F, 0x04, 0x24,                   // movdqu xmm0, [rsp]
    0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x10,             // movdqu xmm1, [rsp+10h]
    0xF3, 0x0F, 0x6F, 0x54, 0x24, 0x20,             // movdqu xmm2, [rsp+20h]
    0xF3, 0x0F, 0x6F, 0x5C, 0x24, 0x30,             // movdqu xmm3, [rsp+30h]
    0x48, 0x83, 0xC4, 0x40,                         // add rsp, 40h
    0x41, 0x59, 0x41, 0x58, 0x5A, 0x59,             // pop r9, r8, dx, cx
    0x50,                                           // push rax
    0x48, 0x8D, 0x05, 0x06, 0, 0, 0,                // lea rax, [rip + 0x06]; ref label3
    0x48, 0x89, 0x44, 0x24, 0x08,                   // mov [rsp+8], rax
    0xC3,                                           // ret
    0x50,                                           // push rax; label3
    0x50,                                           // push rax
    0x4C, 0x8D, 0x44, 0x24, 0x08,                   // lea r8, [rsp+8h]
    0x48, 0x83, 0xEC, 0x10,                         // sub rsp, 10h
    0xF3, 0x0F, 0x7F, 0x04, 0x24,                   // movdqu [rsp], xmm0
    0xB2, 0x01,                                     // mov dl, 1
    0xE9, 0x80, 0xFF, 0xFF, 0xFF,                   // jmp -128; goto label1
};
const std::size_t INTER_ARG_OFF = 0x2A;
const std::size_t INTER_FN_ADDR_OFF = 0x40;

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

std::size_t get_interceptor_opcode_size()
{
    return sizeof(INTER_OPCODE_TEMPLATE);
}

void write_interceptor_opcode(char* buffer, void* inter_fn, void* arg)
{
    memcpy(buffer, INTER_OPCODE_TEMPLATE, sizeof(INTER_OPCODE_TEMPLATE));
    memcpy(buffer + INTER_ARG_OFF, &arg, sizeof(arg));
    memcpy(buffer + INTER_FN_ADDR_OFF, &inter_fn, sizeof(inter_fn));
}
}
