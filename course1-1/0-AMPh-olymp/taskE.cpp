#include <iostream>

static const unsigned TIMEOUT = 20;

int main()
{
    int N;
    std::cin >> N;

    unsigned result = (N / TIMEOUT) + ((N % TIMEOUT) ? 1 : 0);
    std::cout << result << std::endl;
    return 0;
}
