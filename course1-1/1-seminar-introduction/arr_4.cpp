#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstring>

void eratosthenes_sieve (int n, bool *sieve)
{
   std::memset (sieve, true, n);
   sieve[0] = sieve[1] = false;
   for (int i=2; i*i<=n; ++i)   // valid for n < 46340^2 = 2147395600
       if (sieve[i])
           for (int j=i*i; j<=n; j+=i)
               sieve[j] = false;
}

static const int LIMIT = 1300000;
bool sieve[LIMIT];

bool is_composite (int n)
{
    return !sieve[n];
}

int main()
{
    eratosthenes_sieve (LIMIT, sieve);

    int n;
    std::cin >> n;

    std::remove_copy_if    (std::istream_iterator<int>(std::cin),
                         std::istream_iterator<int>(),
                         std::ostream_iterator<int>(std::cout, " "),
                         is_composite);
    std::cout << std::endl;
}
