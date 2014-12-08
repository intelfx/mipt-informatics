#include <stdio.h>
#include <iostream>
#include <set>

std::set<int> squares;

bool check_candidate (int n)
{
    std::set<int>::iterator i, j;
    for (i = squares.begin(); i != squares.end(); ++i) {
        if (*i >= n) {
            break;
        }

        for (j = squares.begin(); j != squares.end(); ++j) {
            if (*i + *j >= n) {
                break;
            }

            int r = n - *i - *j;
            if (r <= 0) {
                break;
            }

            if (squares.count (r)) {
                return true;
            }
        }
    }

    return false;
}

int main()
{
    int N;
    std::cin >> N;

    for (int i = 0;; ++i) {
        int i_sq = i * i;
        if (i_sq > N) {
            break;
        }
        squares.insert (i_sq);
    }

    int result = 0;
    for (int i = 1; i <= N; ++i) {
        if (!check_candidate (i)) {
            ++result;
        }
    }

    std::cout << result << std::endl;
}
