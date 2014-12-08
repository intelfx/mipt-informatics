#include <cmath>
#include <iostream>

int main()
{
    static const double eps = 0.1;

    double a, b, v, t;
    std::cin >> a >> b >> v >> t;

    double p = v * t;
    if ((p > a - eps) && (p < b + eps)) {
        std::cout << "YES" << std::endl;
    } else {
        std::cout << "NO" << std::endl;
    }
}
