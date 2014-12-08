#include <iostream>
#include <iomanip>

int main()
{
    int a, b;
    std::cin >> a >> b;
    while (a--) std::cout << std::setfill('*') << std::setw(b) << "" << std::endl;
}
