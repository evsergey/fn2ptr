#pragma once
#include <cstddef>

namespace fn2ptr
{
std::size_t get_opcode_size();
void write_opcode(char* buffer, const void* fn_addr, void* arg, void** arg_addr);
}
