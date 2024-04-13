#pragma once

#include "mempage.h"

#include <vector>

namespace fn2ptr
{
template<class Proxy>
class fn_factory
{
public:
    fn_factory(size_t capacity = 0) :
        _opcode_size(Proxy::get_opcode_size()),
        _page(capacity * _opcode_size)
    {
        _proxies.resize(_page.size() / _opcode_size);
        for (size_t i = 0; i < _proxies.size(); ++i)
            _proxies[i].create(_page.data() + _opcode_size * i);
        _page.make_executable();
    }

    template<class... Args>
    auto create(Args&&... args)
    {
        const auto result = _proxies[_count].bind(_page.data() + _opcode_size * _count, std::forward<Args>(args)...);
        ++_count;
        return result;
    }

    bool is_full() const { return _proxies.size() == _count; }
    std::size_t size() const { return _count; }
    std::size_t capacity() const { return _proxies.size(); }

private:
    const std::size_t _opcode_size;
    mem_page _page;
    std::vector<Proxy> _proxies;
    size_t _count = 0;
};
}
