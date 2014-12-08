#include <iostream>
#include <iomanip>

int main()
{
    int n;
    std::cin >> n;
    std::cout << std::setfill('*') << std::setw(n) << "" << std::endl;
}
