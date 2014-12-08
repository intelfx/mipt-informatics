#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

int main()
{
    int n, k;
    std::vector<int> data;

    std::cin >> n;
    std::copy (std::istream_iterator<int>(std::cin),
               std::istream_iterator<int>(),
               std::back_inserter(data)); // no std::copy_n() in C++03, sigh
    k = data.back();
    data.pop_back();

    std::vector<int>::iterator iter = std::find (data.begin(), data.end(), k);
    if (iter == data.end()) {
        std::cout << -1 << std::endl;
    } else {
        std::cout << iter - data.begin() << std::endl;
    }
}
