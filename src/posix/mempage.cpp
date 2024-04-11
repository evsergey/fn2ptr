#include "mempage.h"

#include <cstdlib>

#include <unistd.h>
#include <sys/mman.h>

using namespace fn2ptr;

mem_page::mem_page(std::size_t size)
{
    const auto page_size = sysconf(_SC_PAGESIZE);
    _size = page_size;
    if (size > _size)
        _size *= size / _size + std::size_t(size % _size > 0);
    _data = reinterpret_cast<char*>(aligned_alloc(page_size, _size));
    if (!_data)
        _size = 0;
}

void mem_page::release()
{
    mprotect(_data, _size, PROT_READ | PROT_WRITE);
    free(_data);
}

void mem_page::make_executable()
{
    mprotect(_data, _size, PROT_EXEC);
}
