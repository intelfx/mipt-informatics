#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <cmath>

typedef long double fp_t;

int main()
{
    int N;
    std::vector<fp_t> data;

    std::cin >> N;
    std::copy (std::istream_iterator<fp_t>(std::cin),
               std::istream_iterator<fp_t>(),
               std::back_inserter(data));

    fp_t avg = 0, std_dev = 0;

    for (int i = 0; i < data.size(); ++i) {
        avg += data[i];
    }
    avg /= data.size();

    for (int i = 0; i < data.size(); ++i) {
        std_dev += (data[i] - avg) * (data[i] - avg);
    }
    std_dev = sqrtl (std_dev / data.size());

    std::cout << avg << " " << std_dev << std::endl;
}
