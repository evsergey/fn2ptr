#include "asm.h"

#include <cstring>

namespace
{
const unsigned char OPCODE_TEMPLATE[] = {
    0x50,                                           // push rax
    0x51,                                           // push rcx
    0x64, 0x48, 0x8B, 0x0C, 0x25, 0, 0, 0, 0,       // mov rcx, qword ptr fs:[0]
    0x48, 0xB8,                                     // mov rax, <offset>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x48, 0x29, 0xC1,                               // sub rcx, rax
    0x48, 0xB8,                                     // mov rax, <arg>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x48, 0x89, 0x01,                               // mov qword ptr[rcx], rax
    0x58,                                           // pop rax
    0x59,                                           // pop rcx
    0x48, 0xB8,                                     // mov rax, fn_addr
    0, 0, 0, 0, 0, 0, 0, 0,
    0x50,                                           // push rax
    0xC3,                                           // ret
};

const std::size_t ARG_OFFSET_OFF = 0x0D;
const std::size_t ARG_OFF = 0x1A;
const std::size_t FN_ADDR_OFF = 0x29;

const unsigned char INTER_OPCODE_TEMPLATE[] = {
    0x50, 0x50,                                     // push rax, rax
    0x50, 0x57, 0x56, 0x41, 0x52,                   // push rax, rdi, rsi, r10
    0x51, 0x52, 0x41, 0x50, 0x41, 0x51,             // push cx, dx, r8, r9
    0x48, 0x8D, 0x54, 0x24, 0x50,                   // lea rdx, [rsp+50h]
    0x48, 0x81, 0xEC, 0x80, 0, 0, 0,                // sub rsp, 80h
    0xF3, 0x0F, 0x7F, 0x04, 0x24,                   // movdqu [rsp], xmm0
    0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x10,             // movdqu [rsp+10h], xmm1
    0xF3, 0x0F, 0x7F, 0x54, 0x24, 0x20,             // movdqu [rsp+20h], xmm2
    0xF3, 0x0F, 0x7F, 0x5C, 0x24, 0x30,             // movdqu [rsp+30h], xmm3
    0xF3, 0x0F, 0x7F, 0x64, 0x24, 0x40,             // movdqu [rsp+40h], xmm4
    0xF3, 0x0F, 0x7F, 0x6C, 0x24, 0x50,             // movdqu [rsp+50h], xmm5
    0xF3, 0x0F, 0x7F, 0x74, 0x24, 0x60,             // movdqu [rsp+60h], xmm6
    0xF3, 0x0F, 0x7F, 0x7C, 0x24, 0x70,             // movdqu [rsp+70h], xmm7
    0x66, 0x31, 0xF6,                               // xor si, si
    0x48, 0xB9,                                     // mov rcx, <arg>;  label1
    0, 0, 0, 0, 0, 0, 0, 0,
    0x48, 0x89, 0xCF,                               // mov rdi, rcx
    0x48, 0x83, 0xEC, 0x20,                         // sub rsp, 32
    0x48, 0x8D, 0x05, 0x0D, 0, 0, 0,                // lea rax, [rip + 0x0D]; ref label2
    0x50,                                           // push rax
    0x48, 0xB8,                                     // mov rax, <inter>
    0, 0, 0, 0, 0, 0, 0, 0,
    0x50,                                           // push rax
    0xC3,                                           // ret
    0x48, 0x83, 0xC4, 0x20,                         // add rsp, 32; label2
    0x48, 0x09, 0xC0,                               // or rax, rax
    0x0F, 0x85, 0x12, 0, 0, 0,                      // jnz 0x12
    0xF3, 0x0F, 0x6F, 0x04, 0x24,                   // movdqu xmm0, [rsp]
    0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x10,             // movdqu xmm1, [rsp + 10h]
    0x48, 0x83, 0xC4, 0x20,                         // add rsp, 20h
    0x5A,                                           // pop rdx
    0x58,                                           // pop rax
    0xC3,                                           // ret
    0xF3, 0x0F, 0x6F, 0x04, 0x24,                   // movdqu xmm0, [rsp]
    0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x10,             // movdqu xmm1, [rsp+10h]
    0xF3, 0x0F, 0x6F, 0x54, 0x24, 0x20,             // movdqu xmm2, [rsp+20h]
    0xF3, 0x0F, 0x6F, 0x5C, 0x24, 0x30,             // movdqu xmm3, [rsp+30h]
    0xF3, 0x0F, 0x6F, 0x64, 0x24, 0x40,             // movdqu xmm4, [rsp+40h]
    0xF3, 0x0F, 0x6F, 0x6C, 0x24, 0x50,             // movdqu xmm5, [rsp+50h]
    0xF3, 0x0F, 0x6F, 0x74, 0x24, 0x60,             // movdqu xmm6, [rsp+60h]
    0xF3, 0x0F, 0x6F, 0x7C, 0x24, 0x70,             // movdqu xmm7, [rsp+70h]
    0x48, 0x81, 0xC4, 0x80, 0, 0, 0,                // add rsp, 80h
    0x41, 0x59, 0x41, 0x58, 0x5A, 0x59,             // pop r9, r8, dx, cx
    0x41, 0x5A, 0x5E, 0x5F,                         // pop r10, rsi, rdi
    0x48, 0x89, 0x44, 0x24, 0x08,                   // mov [rsp+8], rax
    0x48, 0x8D, 0x05, 0x07, 0, 0, 0,                // lea rax, [rip + 0x07]; ref label3
    0x48, 0x89, 0x44, 0x24, 0x10,                   // mov [rsp+10h], rax
    0x58,                                           // pop rax
    0xC3,                                           // ret
    0x50,                                           // push rax; label3
    0x50, 0x52,                                     // push rax, rdx
    0x48, 0x8D, 0x54, 0x24, 0x10,                   // lea rdx, [rsp+10h]
    0x48, 0x83, 0xEC, 0x20,                         // sub rsp, 20h
    0xF3, 0x0F, 0x7F, 0x04, 0x24,                   // movdqu [rsp], xmm0
    0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x10,             // movdqu [rsp+10h], xmm1
    0x66, 0xBE, 0x01, 0x00,                         // mov si, 1
    0xE9, 0x49, 0xFF, 0xFF, 0xFF,                   // jmp -183; goto label1
};

const std::size_t INTER_ARG_OFF = 0x4D;
const std::size_t INTER_FN_ADDR_OFF = 0x66;


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
