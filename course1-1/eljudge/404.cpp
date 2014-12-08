#include <iostream>
#include <algorithm>

typedef unsigned long long data_t;
static const size_t LIMIT = 1025;

// The stack is not enough, and there are no multidimensional dynamic arrays.
data_t C_temp[LIMIT /* k */][LIMIT /* n */];

data_t C (unsigned N, unsigned K)
{
    for (unsigned k = 0; k <= K; ++k) {
        for (unsigned n = k; n <= N; ++n) {
            if ((k == 0) || (k == n)) {
                C_temp[k][n] = 1;
            } else {
                C_temp[k][n] = C_temp[k-1][n-1] + C_temp[k][n-1];
            }
        }
    }

    return C_temp[K][N];
}

int main()
{
    unsigned n, k;
    std::cin >> n >> k;
    std::cout << C (n, k) << std::endl;
    return 0;
}
