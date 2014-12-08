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

    int idx = data.size() - 1;
    for (int i = 0; i < data.size(); ++i) {
        std::cout << data[idx] << " ";
        idx = (idx + 1)    % data.size();
    }
    std::cout << std::endl;
}
