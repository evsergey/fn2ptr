#include "fn2ptr.h"

#include <iostream>

using namespace fn2ptr;

int main()
{
    func_ptr_factory<int, int> factory{ 2 };
    if (factory.capacity() < 2)
    {
        std::cout << "Couldn't create factory" << std::endl;
        return EXIT_FAILURE;
    }

    int a = 100500;
    
    int (*f)(int) = factory.create(
        [&a](int b) {
        std::cout << "a=" << a << ", b=" << b << std::endl;
        return a + b;
        });

    int (*F)(int) = factory.create(
        [a](int b) {
            std::cout << "A=" << a << ", B=" << b << std::endl;
            return a - b;
        });

    int q = f(123);
    std::cout << "q=" << q << std::endl;
    F(123);
    a = 333;
    f(234);
    F(234);

    return EXIT_SUCCESS;
}
