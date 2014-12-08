#include <iostream>

int main()
{
    unsigned N, K;
    std::cin >> N >> K;

    unsigned total = N * (K + 1);
    std::cout << ((total <= 60) ? "YES" : "NO") << std::endl;

    return 0;
}
