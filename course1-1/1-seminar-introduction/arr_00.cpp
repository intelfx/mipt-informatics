#include <iostream>
#include <iterator>
#include <vector>

int main()
{
    int n;
    std::vector<int> data;

    std::cin >> n;
    std::copy (std::istream_iterator<int>(std::cin),
               std::istream_iterator<int>(),
               std::back_inserter(data));

    std::copy (data.begin(), data.end(), std::ostream_iterator<int>(std::cout, " "));
    std::copy (data.begin(), data.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
}
