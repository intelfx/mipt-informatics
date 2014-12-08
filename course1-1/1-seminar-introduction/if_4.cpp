#include <iostream>

int main()
{
    int x;
    std::cin >> x;
    std::cout << ((x < 37 || x >= 146) ? "YES" : "NO") << std::endl;
}
