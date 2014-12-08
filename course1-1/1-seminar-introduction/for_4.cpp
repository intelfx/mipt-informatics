#include <iostream>

int main()
{
    int n, fact = 1;
    std::cin >> n;
    for (int i = 2; i <= n; ++i) fact *= i;
    std::cout << fact << std::endl;
}
