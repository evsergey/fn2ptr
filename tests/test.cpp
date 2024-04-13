#include "fn2ptr.h"

#include <iostream>

using namespace fn2ptr;

int ff(int x, int y)
{
    std::cout << "x=" << x << ", y=" << y << std::endl;
    return x + y;
}

int main()
{
    lambda_proxy_factory<int, int> lp_factory{ 2 };
    if (lp_factory.capacity() < 2)
    {
        std::cout << "Couldn't create factory" << std::endl;
        return EXIT_FAILURE;
    }
    fnptr_proxy_factory fp_factory{ 1 };
    if (fp_factory.capacity() < 1)
    {
        std::cout << "Couldn't create factory" << std::endl;
        return EXIT_FAILURE;
    }

    int a = 100500;
    
    int (*f)(int) = lp_factory.create(
        [&a](int b) {
        std::cout << "a=" << a << ", b=" << b << std::endl;
        return a + b;
        });

    int (*F)(int) = lp_factory.create(
        [a](int b) {
            std::cout << "A=" << a << ", B=" << b << std::endl;
            return a - b;
        });

    auto g = fp_factory.create(&ff,
        [&a]() {
            std::cout << "Pre call, a=" << a << std::endl;
        },
        [&a]() {
            std::cout << "Post call, a=" << a << std::endl;
        }
    );
    int q = f(123);
    std::cout << "q=" << q << std::endl;
    F(123);
    a = 333;
    f(234);
    F(234);


    q = g(1, 2);
    std::cout << "q=" << q << std::endl;
    a = 3;
    q = g(4, 5);
    std::cout << "q=" << q << std::endl;

    return EXIT_SUCCESS;
}
