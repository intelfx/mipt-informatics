#include <cmath>
#include <iostream>

int main()
{
    static const double eps = 0.1;

    double a, b;
    std::cin >> a >> b;

    std::cout << (fabsl (a - b) < eps ? "YES" : "NO") << std::endl;
}
