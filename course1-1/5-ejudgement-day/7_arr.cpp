#include <iostream>
#include <iterator>
#include <algorithm>
#include <deque>

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

    std::deque<int> data;
    std::copy (std::istream_iterator<int> (std::cin),
               std::istream_iterator<int>(),
               std::back_inserter (data));

    while (!data.empty()) {
        if (&data.front() == &data.back()) {
            std::cout << data.front() << " ";
            data.pop_front();
        } else {
            std::cout << std::min (data.front(), data.back()) << " "
                      << std::max (data.front(), data.back()) << " ";
            data.pop_front();
            data.pop_back();
        }
    }
    std::cout << std::endl;
}
