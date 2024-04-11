#pragma once
#include "asm.h"
#include "mempage.h"

#include <functional>
#include <vector>

namespace fn2ptr
{
template<class R, class... Args>
class func_ptr_factory
{
    class func_proxy
    {
    public:
        void create(char* buffer) const
        {
            write_opcode(buffer, (void*)func_proxy::call, (void*)this, (void**)func_proxy::current());
        }

        template<class F>
        void operator=(F&& fn)
        {
            _fn = std::forward<F>(fn);
        }

    private:
        std::function<R(Args...)> _fn;
        static func_proxy** current()
        {
            static thread_local func_proxy* _current = nullptr;
            return &_current;
        }
        static R call(Args... args)
        {
            return (*current())->_fn(args...);
        }
    };

public:
    using func_ptr_t = R(*)(Args...);

    func_ptr_factory(size_t capacity = 0):
        _page(capacity * get_opcode_size())
    {
        const auto opcode_size = get_opcode_size();
        _proxies.resize(_page.size() / opcode_size);
        for (size_t i = 0; i < _proxies.size(); ++i)
            _proxies[i].create(_page.data() + opcode_size * i);
        _page.make_executable();
    }

    template<class F>
    func_ptr_t create(F&& fn)
    {
        _proxies[_count] = std::forward<F>(fn);
        const auto opcode_size = get_opcode_size();
        auto result = reinterpret_cast<func_ptr_t>(_page.data() + opcode_size * _count);
        ++_count;
        return result;
    }

    bool is_full() const { return _proxies.size() == _count; }
    std::size_t size() const { return _count; }
    std::size_t capacity() const { return _proxies.size(); }

private:
    mem_page _page;
    std::vector<func_proxy> _proxies;
    size_t _count = 0;
};
}
