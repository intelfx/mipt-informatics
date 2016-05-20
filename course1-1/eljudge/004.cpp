#include <stdio.h>
#include <stdlib.h>

struct athlet
{
    int m;
    int s;
};
/*
int cmp(const struct athlet *a, const struct athlet *b)
    {
        if (a -> s - b -> s > 0)
            return 1;
        if (a -> s - b -> s < 0)
            return -1;
        return 0;
    }
*/
int cmp(const void* x, const void* y)
    {
        struct athlet* a = (struct athlet*)x;
        struct athlet* b = (struct athlet*)y;
        if (a -> s - b -> s > 0)
            return 1;
        if (a -> s - b -> s < 0)
            return -1;
        return 0;
    }
int main()
{
    int i, n;
    scanf("%d", &n);
    struct athlet athletes[n];
    for (i = 0; i < n; i++)
    {
        scanf("%d %d", &athletes[i].m, &athletes[i].s);
    }

    struct athlet *c;
    c = &athletes[0];
    qsort(c, n, sizeof(struct athlet), cmp);

    int m_sum = 0;
    int k = 0;
    for (i = 0; i < n; i++)
    {
        if (athletes[i].s >= m_sum)
        {
            m_sum += athletes[i].m;
            k++;
        }
    }
    printf("%d", k);
    return 0;
}