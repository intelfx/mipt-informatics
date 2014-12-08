#include <cstdio>

int main()
{
    int nesting = 0, ch;
    while ((ch = getchar()) >= 0) {
        switch (ch) {
        case '(': ++nesting; break;
        case ')': --nesting; break;
        }
        if (nesting < 0) {
            break;
        }
    }

    puts (nesting ? "NO" : "YES");
}
