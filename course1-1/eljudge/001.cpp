#include <iostream>
#include <iterator>
#include <climits>
#include <algorithm>

int main()
{
    int max = INT_MIN;

    while (!std::cin.eof()) {
        int x;
        std::cin >> x;
        max = std::max (max, x);
    }

    std::cout << max << std::endl;
}
