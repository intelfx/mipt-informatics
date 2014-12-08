#include <iostream>
#include <iterator>
#include <vector>

int main()
{
    int n, k;
    std::vector<int> data;

    std::cin >> n;
    std::copy (std::istream_iterator<int>(std::cin),
               std::istream_iterator<int>(),
               std::back_inserter(data)); // again, no std::copy_n(), poor me...
    k = data.back();
    data.pop_back();

    int idx = (data.size() - k);
    while (idx < 0) idx += data.size();
    idx %= data.size();
    for (int i = 0; i < data.size(); ++i) {
        std::cout << data[idx] << " ";
        idx = (idx + 1)    % data.size();
    }
    std::cout << std::endl;
}
