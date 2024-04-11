#pragma once
#include <cstddef>

namespace fn2ptr
{
class mem_page
{
public:
    mem_page(std::size_t size = 0);
    mem_page(mem_page&& page) noexcept
    {
        _data = page._data;
        _size = page._size;
        page._size = 0;
    }

    ~mem_page()
    {
        if (_size)
            release();
    }

    mem_page& operator=(mem_page&& page) noexcept
    {
        if (_size)
            release();
        _data = page._data;
        _size = page._size;
        page._size = 0;
        return *this;
    }

    char* data() const { return _data; }
    std::size_t size() const { return _size; }
    void make_executable();

private:
    char* _data;
    std::size_t _size;
    void release();
};
}
