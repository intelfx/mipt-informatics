#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
long double data[10000000];
int __cmp_double(const void* __1, const void* __2)
{
    if(*(long double*)__1 > *(long double*)__2)
        return 1;
    if(*(long double*)__1 < *(long double*)__2)
        return -1;
    return 0;
}

int main(void)
{
    size_t iter = 0, i = 0;
    while(scanf("%Lf", &data[iter]) == 1)
    {
        iter++;
    }
    qsort(data, iter, sizeof(long double), __cmp_double);
    long double result = 0.0;
    for(i = 0; i < iter; i++)
        result += data[i];

    printf("%.20Lg\n", result);
    return 0;
} 
