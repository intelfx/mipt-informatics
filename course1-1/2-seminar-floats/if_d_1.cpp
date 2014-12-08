#include <cmath>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>

int main()
{
    double a, b, c;
    std::cin >> a >> b >> c;

    std::vector < double > solutions;
    bool is_R = false;

    if (a != 0)
    {
        double D = b*b - 4*a*c;
        if (D > 0)
        {
            solutions.push_back ((-b - sqrtl (D)) / (2*a));
            solutions.push_back ((-b + sqrtl (D)) / (2*a));
        } else if (D == 0)
        {
            solutions.push_back (-b / (2*a));
        }
    } else
    {
        if (b != 0)
        {
            solutions.push_back (-c / b);
        } else if (c == 0)
        {
            is_R = true;
        }
    }

    if (is_R)
    {
        std::cout << "R" << std::endl;
    } else if (solutions.empty())
    {
        std::cout << "NO" << std::endl;
    } else
    {
        std::sort (solutions.begin(), solutions.end());
        for (size_t i = 0; i < solutions.size(); ++i)
        {
            if (solutions[i] == -0)
            {
                solutions[i] = 0;
            }
        }

        std::copy (solutions.begin(), solutions.end(),
                   std::ostream_iterator < double > (std::cout, " "));
        std::cout << std::endl;
    }
}
