#pragma once
#include "asm.h"

#include <exception>
#include <functional>
#include <vector>

namespace fn2ptr
{
    class fnptr_proxy
    {
    public:
        void create(char* buffer) const
        {
            write_interceptor_opcode(buffer, (void*)&fnptr_proxy::interceptor, (void*)this);
        }

        template<class Pre, class Post, class R, class... Args>
        auto bind(char* buffer, R(*fn)(Args... args) noexcept, Pre&& pre, Post&& post)
        {
            _fn = (void*)fn;
            _pre = std::forward<Pre>(pre);
            _post = std::forward<Post>(post);
            return reinterpret_cast<decltype(fn)>(buffer);
        }

        static std::size_t get_opcode_size()
        {
            return get_interceptor_opcode_size();
        }

    private:
        void* _fn;
        std::function<void()> _pre;
        std::function<void()> _post;
        std::vector<void*> _rets;

        static void* interceptor(fnptr_proxy* _this, char f, void** pret) noexcept
        {
            return _this->intercept(f, pret);
        }

        void* intercept(char f, void** pret) noexcept
        {
            try
            {
                if (f)
                {
                    *pret = _rets.back();
                    _rets.pop_back();
                    if (_post)
                        _post();
                    return nullptr;
                }
                _rets.push_back(*pret);
                if (_pre)
                    _pre();
                return _fn;
            }
            catch (...)
            {
                std::terminate();
            }
        }
    };
}
