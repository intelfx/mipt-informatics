#include <iostream>

int main()
{
    int N, result = 0;
    std::cin >> N;

    int x[4];

    for (x[3] = 0; 4 * x[3] <= N; ++x[3]) {
        for (x[2] = 0; 3 * x[2] + 4 * x[3] <= N; ++x[2]) {
            for (x[1] = 0; 2 * x[1] + 3 * x[2] + 4 * x[3] <= N; ++x[1]) {
                ++result;
            }
        }
    }

    std::cout << result << std::endl;
}
