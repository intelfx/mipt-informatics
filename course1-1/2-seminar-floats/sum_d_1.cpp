#include <iostream>
#include <cmath>

typedef double fp_t;

int main()
{
    fp_t center[2] = { };
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            fp_t input;
            std::cin >> input;
            center[j] += input;
        }
    }

    std::cout << center[0] / 3 << " " << center[1] / 3 << std::endl;
}
