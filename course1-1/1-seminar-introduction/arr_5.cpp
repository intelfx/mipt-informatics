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
           for (int j=i*i; j<n; j+=i)
               sieve[j] = false;
}

int main()
{

    int n;
    std::cin >> n;

    bool* sieve = new bool[n+1];
    eratosthenes_sieve (n+1, sieve);

    while (n > 1) {
        for (int i = 0;; ++i) {
            if (sieve[i] && !(n % i)) {
                std::cout << i << " ";
                n /= i;
                break;
            }
        }
    }
    std::cout << std::endl;

    delete[] sieve;
}
