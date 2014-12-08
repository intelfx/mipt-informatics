#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

bool is_even (int x)
{
    return !(x % 2);
}

bool is_odd (int x)
{
    return (x % 2);
}

int main()
{
    int n;
    std::vector<int> data;

    std::cin >> n;
    std::copy (std::istream_iterator<int>(std::cin),
               std::istream_iterator<int>(),
               std::back_inserter(data));

    std::remove_copy_if (data.begin(), data.end(), std::ostream_iterator<int>(std::cout, " "), is_odd);
    std::cout << std::endl;
    std::remove_copy_if (data.begin(), data.end(), std::ostream_iterator<int>(std::cout, " "), is_even);
    std::cout << std::endl;
}
