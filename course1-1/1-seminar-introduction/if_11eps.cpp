#include <cmath>
#include <iostream>

int main()
{
    static const double eps = 0.1, c_ref = 5;

    double a, b;
    std::cin >> a >> b;

    double c = sqrtl (a*a + b*b);
    std::cout << (fabsl (c - c_ref) < eps ? "YES" : "NO") << std::endl;
}
