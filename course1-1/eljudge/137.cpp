#include <locale.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <iterator>
#include <stack>
using namespace std;
int main(void)
{
    unsigned long long int n, m;
    cin >> n >> m;
    unsigned long long int a = 1;
    unsigned long long int b = 1;
    unsigned long long int c = 1;
    unsigned long long int d = 0;
    unsigned long long int rc = 0;
    unsigned long long int rd = 1;
    unsigned long long int ta;
    unsigned long long int tb;
    unsigned long long int tc;
    while( n > 0 )
    {
        if(n % 2 == 1)
        {
            tc = rc;
            rc = (rc * a + rd * c) % m;
            rd = (tc * b + rd * d) % m;
        }

        ta = a;
        tb = b;
        tc = c;
        a = (a * a + b * c) % m;
        b = (ta * b + b * d) % m;
        c = (c * ta + d * c) % m;
        d = (tc * tb + d * d) % m;
        n = n / 2;
    }
    cout << rc << endl;

    return 0;
}
