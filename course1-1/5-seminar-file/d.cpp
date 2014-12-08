#include <fstream>
#include <iostream>

int get_divisor_count (long value)
{
    int count = 0; // 1 and value
    for (long i = 1; i*i <= value; ++i) {
        if (!(value % i)) {
            ++count;
            if (i != value / i) {
                ++count;
            }
        }
    }
    return count;
}

int main()
{
    std::ifstream in ("input.txt");
    std::ofstream out ("output.txt");
    int N;
    in >> N;

    long long value = 1;
    for (int i = 2;; value += i, ++i) {
        if (get_divisor_count (value) > N) {
            out << value << std::endl;
            return 0;
        }
    }
}
