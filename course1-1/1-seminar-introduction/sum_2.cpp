#include <iostream>
#include <cmath>

int main()
{
    int k, n;
    std::cin >> k >> n;
    std::cout << 18 + (n - k) / 33 + ((n - k) % 33 ? 1 : 0) << std::endl;
}
