#include <iostream>
#include <iterator>
#include <algorithm>
#include <numeric>

int sum = 0;

void sum_if_pred (int value)
{
    if (!(value % 3)) {
        sum += value;
    }
}

int main()
{
    int N;
    std::cin >> N;

    std::for_each (std::istream_iterator<int> (std::cin),
                   std::istream_iterator<int>(),
                   sum_if_pred);

    std::cout << sum << std::endl;
}
