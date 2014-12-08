#include <iostream>

int main()
{
    unsigned N;
    std::cin >> N;

    unsigned long result = 0;

    while (N--) {
        unsigned mark, count;
        std::cin >> mark >> count;
        result += mark * count;
    }

    std::cout << result << std::endl;
    return 0;
}
