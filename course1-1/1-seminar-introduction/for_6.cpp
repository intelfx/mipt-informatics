#include <cstdio>

int main()
{
    int digit, ch, result = 0;
    scanf ("%d ", &digit);
    
    while ((ch = getchar()) >= 0) { if (ch - '0' == digit) ++result; }
    printf ("%d\n", result);
}
