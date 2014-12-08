#include <ctype.h>

int my_strnicmp (const char* s1, const char* s2, size_t n)
{
    char c1, c2;
    while ((c1 = tolower (*s1), c2 = tolower (*s2), c1 && c2 && (c1 == c2))) {
        if (!--n) {
            return 0;
        }
        ++s1;
        ++s2;
    }

    if ((unsigned)c1 < (unsigned)c2) return -1;
    else if ((unsigned)c1 > (unsigned)c2) return 1;
    else return 0;
}
