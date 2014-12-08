#include <iostream>

int main()
{
    unsigned a, n;
    std::cin >> a >> n;

    unsigned output = a;
    for (unsigned i = 1; i < n; ++i) output *= a;
    std::cout << output << std::endl;
}
