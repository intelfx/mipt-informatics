#include <iostream>
#include <cmath>
#include <utility>

typedef double fp_t;

fp_t func (fp_t arg)
{
    return sin (arg*arg);
}

static const fp_t eps = 1e-6;

int main()
{
    fp_t a, b;
    bool changesign = false;
    std::cin >> a >> b;
    if (b < a) {
        std::swap (a, b);
        changesign = true;
    }

    static const int STEPS = 10000;
    fp_t step = (b - a) / STEPS;

    if (step < eps) {
        std::cout << 0 << std::endl;
        return 0;
    }

    fp_t arg = a, last = func (arg), cur, out = 0;
    while ((arg += step) < b + eps) {
        cur = func (arg);
        out += step * last;
        out += step * (cur - last) / 2;
        last = cur;
    }

    if (changesign) {
        out = -out;
    }

    std::cout << out << std::endl;
}
