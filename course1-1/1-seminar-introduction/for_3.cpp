#include <iostream>
#include <iterator>
#include <algorithm>

bool is_even (int x)
{
    return !(x % 2);
}

int main()
{
    int n;
    std::cin >> n;

    std::remove_copy_if (std::istream_iterator<int>(std::cin),
                         std::istream_iterator<int>(),
                         std::ostream_iterator<int>(std::cout, " "),
                         is_even);
    std::cout << std::endl;
}
