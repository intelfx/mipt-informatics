#include <iostream>

typedef unsigned long data_t;

int main()
{
    data_t N;
    std::cin >> N;

    data_t order = 1, floor = 1;
    data_t floor_in_order = 0, nr_in_floor = 0;

    while (--N) {
        ++nr_in_floor;
        if (nr_in_floor >= order) {
            // next floor
            ++floor;
            ++floor_in_order;
            nr_in_floor = 0;

            if (floor_in_order >= order) {
                // next order
                ++order;
                floor_in_order = 0;
            }
        }
    }

    std::cout << floor << " " << nr_in_floor + 1 << std::endl;
}
