#pragma once
#include "asm.h"

#include <functional>

namespace fn2ptr
{
template<class R, class... Args>
class lambda_proxy
{
    using func_ptr_t = R(*)(Args...);
public:
    void create(char* buffer) const
    {
        write_opcode(buffer, (void*)&call, (void*)this, (void**)current());
    }

    template<class F>
    func_ptr_t bind(const char* buffer, F&& fn)
    {
        _fn = std::forward<F>(fn);
        return reinterpret_cast<func_ptr_t>(buffer);
    }

    static std::size_t get_opcode_size()
    {
        return fn2ptr::get_opcode_size();
    }

private:
    std::function<R(Args...)> _fn;
    static lambda_proxy** current()
    {
        static thread_local lambda_proxy* _current = nullptr;
        return &_current;
    }
    static R call(Args... args)
    {
        return (*current())->_fn(args...);
    }
};
}
