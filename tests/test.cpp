#include "fn2ptr.h"

#include <catch2/catch_all.hpp>

using namespace fn2ptr;

int ff(int x, int y, int z, int w, int p, int q) noexcept
{
    return x + y + z + w + p + q;
}

double gg(double x, double y, double z, double w, double p, double q) noexcept
{
    return x + y + z + w + p + q;
}

TEST_CASE("Hooks", "[fn2ptr]")
{
    fnptr_proxy_factory fp_factory{ 2 };
    REQUIRE(fp_factory.capacity() >= 2);

    bool pre_called = false;
    bool post_called = false;
    auto f = fp_factory.create(&ff,
        [&pre_called]() noexcept {
            int r = ff(2, 3, 4, 5, 6, 7);
            REQUIRE(r == 2 + 3 + 4 + 5 + 6 + 7);
            pre_called = true;
        },
        [&post_called]() noexcept {
            int r = ff(2, 3, 4, 5, 6, 7);
            REQUIRE(r == 2 + 3 + 4 + 5 + 6 + 7);
            post_called = true;
        }
    );

    auto g = fp_factory.create(&gg,
        [&pre_called]() noexcept {
            double r = gg(2, 3, 4, 5, 6, 7);
            REQUIRE(r == 2 + 3 + 4 + 5 + 6 + 7);
            pre_called = true;
        },
        [&post_called]() noexcept {
            double r = gg(2, 3, 4, 5, 6, 7);
            REQUIRE(r == 2 + 3 + 4 + 5 + 6 + 7);
            post_called = true;
        }
    );

    int result = f(1, 2, 3, 4, 5, 6);
    REQUIRE(result == 1 + 2 + 3 + 4 + 5 + 6);
    REQUIRE(pre_called);
    REQUIRE(post_called);

    pre_called = post_called = false;

    double fresult = g(1., 2., 3., 4., 5., 6.);
    REQUIRE(fresult == 1. + 2. + 3. + 4. + 5. + 6.);
    REQUIRE(pre_called);
    REQUIRE(post_called);
}

TEST_CASE("lambda-ptr", "[fn2ptr]")
{
    lambda_proxy_factory<int, int> lp_factory{ 2 };
    REQUIRE(lp_factory.capacity() >= 2);

    int a = 100500;
    int (*f)(int) = lp_factory.create(
        [&a](int b) noexcept {
            return a + b;
        });

    int (*g)(int) = lp_factory.create(
        [a](int b) noexcept {
            return a - b;
        });

    int result = f(123);
    REQUIRE(result == 100500 + 123);

    result = g(123);
    REQUIRE(result == 100500 - 123);

    a = 10;
    result = f(5);
    REQUIRE(result == 10 + 5);

    result = g(4);
    REQUIRE(result == 100500 - 4);
}
