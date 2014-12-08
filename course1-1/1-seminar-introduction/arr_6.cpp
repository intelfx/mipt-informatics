#include <iostream>
#include <iterator>
#include <vector>

static const int LIMIT = 15;

int main()
{
    int n;
    std::cin >> n;

    int data[LIMIT][LIMIT];

    for (int i = 0; i < LIMIT; ++i) {
        data[i][0] = 1;
        data[0][i] = 1;
    }

    for (int i = 1; i < LIMIT; ++i) {
        for (int j = 1; j < LIMIT; ++j) {
            data[i][j] = data[i-1][j] + data[i][j-1];
        }
    }

    for (int i = 0; i <= n; ++i) {
        std::cout << data[i][n - i] << " ";
    }
    std::cout << std::endl;
}
