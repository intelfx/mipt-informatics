#include <iostream>

bool is_leap (int year)
{
    return (!(year % 400)) || (!(year % 4) && (year % 100));
}

int main()
{
    int year;
    std::cin >> year;
    std::cout << (is_leap (year) ? "YES" : "NO") << std::endl;
}
