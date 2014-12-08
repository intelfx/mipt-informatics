#include <iostream>
#include <string>

int main()
{
    size_t N, op_count = 0;
    std::cin >> N;

    while (N != 0) {
        if (N == 3) {
            --N;
        } else {
            if (!(N % 2)) {
                N /= 2;
            } else {
                if (!((N - 1) % 4)) {
                    --N;
                } else { /* if (!((N + 1) % 4)) */
                    ++N;
                }
            }
        }
        ++op_count;
    }

    std::cout << op_count << std::endl;
    return 0;
}
