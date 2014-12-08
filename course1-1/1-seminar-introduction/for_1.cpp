#include <iostream>
#include <iterator>
#include <numeric>

int main()
{
    int n;
    std::cin >> n;

    int sum = std::accumulate (std::istream_iterator<int>(std::cin),
                               std::istream_iterator<int>(),
                               0);
    std::cout << sum << std::endl;
}
