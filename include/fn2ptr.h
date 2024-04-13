#pragma once
#include "fn_factory.h"
#include "lambda_proxy.h"
#include "fnptr_proxy.h"

namespace fn2ptr
{
template<class R, class... Args>
using lambda_proxy_factory = fn_factory<lambda_proxy<R, Args...>>;
using fnptr_proxy_factory = fn_factory<fnptr_proxy>;
}
