#include <iostream>

static const double eps = 1e-9;

int main()
{
    double x, y;
    std::cin >> x >> y;

    if ((y >= (1 + x*x) - eps) ||
        (y <= (-2 - x*x) + eps)) {
        std::cout << "YES" << std::endl;
    } else {
        std::cout << "NO" << std::endl;
    }
}
