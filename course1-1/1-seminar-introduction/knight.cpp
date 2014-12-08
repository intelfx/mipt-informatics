#include <iostream>
#include <iterator>

int main()
{
    int N;
    std::vector<int> data;

    std::cin >> N;
    std::copy (std::istream_iterator<int>(std::cin),
               std::istream_iterator<int>(),
               std::back_inserter(data));

    
}
