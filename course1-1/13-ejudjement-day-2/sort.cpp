#include <algorithm>
#include <vector>
#include <iostream>
#include <cmath>
#include <iterator>
#include <iomanip>

bool fp_cmp (float a, float b)
{
    static const float eps = 1e-9;
    return fabs (a - b) < eps;
}

struct Entry
{
    float height, weight;

    bool operator< (const Entry& rhs) const
    {
        if (fp_cmp (weight, rhs.weight)) {
            return height > rhs.height;
        } else {
            return weight < rhs.weight;
        }
    }
};

std::istream& operator>> (std::istream& in, Entry& e)
{
    in >> e.height >> e.weight;
    return in;
}

std::ostream& operator<< (std::ostream& out, const Entry& e)
{
    out << std::fixed << std::setprecision (2) << e.height << " "
        << std::fixed << std::setprecision (3) << e.weight;
    return out;
}

int main()
{
    size_t N;
    std::cin >> N;

    std::vector<Entry> entries;
    entries.reserve (N);
    std::copy (std::istream_iterator<Entry> (std::cin),
               std::istream_iterator<Entry>(),
               std::back_inserter (entries));

    std::sort (entries.begin(), entries.end());

    std::copy (entries.begin(),
               entries.end(),
               std::ostream_iterator<Entry> (std::cout, "\n"));
}
