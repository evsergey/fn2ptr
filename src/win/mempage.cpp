#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "mempage.h"

using namespace fn2ptr;

mem_page::mem_page(std::size_t size)
{
    SYSTEM_INFO sinfo;
    GetSystemInfo(&sinfo);
    _size = sinfo.dwPageSize;
    if (size > _size)
        _size *= size / _size + std::size_t(size % _size > 0);
    _data = reinterpret_cast<char*>(VirtualAlloc(NULL, _size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!_data)
        _size = 0;
}

void mem_page::release()
{
    VirtualFree(_data, 0, MEM_RELEASE);
}

void mem_page::make_executable()
{
    DWORD old;
    VirtualProtect(_data, _size, PAGE_EXECUTE, &old);
}
