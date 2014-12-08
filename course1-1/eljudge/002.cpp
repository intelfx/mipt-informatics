#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>

void read_set (std::set<int>& dest)
{
    for (;;) {
        int x;
        std::cin >> x;

        if (x == -1) {
            break;
        }
        dest.insert (x);
    }
}

int main()
{
    std::set<int> A, B;
    read_set (A);
    read_set (B);
    std::set_intersection (A.begin(), A.end(), B.begin(), B.end(),
                           std::ostream_iterator<int> (std::cout, " "));
    std::cout << std::endl;
}
