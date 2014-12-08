#include <iostream>
#include <cmath>
#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

int main()
{
    int N;
    std::cin >> N;

    if (N < 2) {
        std::cout << "1" << std::endl;
        return 0;
    }

    cpp_int boundary = 1;
    for (int i = 1; i < N; ++i) {
        boundary *= 10;
    }

    cpp_int fibp = 1, fibpp = 1;
    int current_idx = 2;

    for (;;) {
        cpp_int fib = fibp + fibpp;
        ++current_idx;

        if (fib >= boundary) {
            break;
        }

        fibpp = fibp;
        fibp = fib;
    }

    std::cout << current_idx << std::endl;
}
