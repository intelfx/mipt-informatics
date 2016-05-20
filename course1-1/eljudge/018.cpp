#include <stdio.h>

int gcd(int a, int b)
{
    int buff;
    while(b != 0)
    {
        buff = a % b;
        a = b;
        b = buff;
    }
    return a;
}
int main(void)
{
    int n, gc, i, j;
    scanf("%d", &n);
    int count = 0;
    for(i = 2; i <= n; i++)
    {
        for(j = 1; j < i; j++)
            if(gcd(i, j) == 1)
                count++;
    }
    int res = 0, ct = 1;
    while(ct < count)
    {
        res++;
        ct *= 2;
    }
    printf("%d\n", res);
    return 0;
}