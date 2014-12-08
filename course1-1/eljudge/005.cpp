#include <stdio.h>
#include <stdlib.h>

int main()
{
    double output = 0;
    int depth = 1;
    char bracebuf[2];

    do {
        int value;

        if (scanf (" %1[()]", bracebuf) > 0) {
            switch (bracebuf[0]) {
            case '(': depth *= 2; break;
            case ')': depth /= 2; break;
            default: abort();
            }
        } else if (scanf (" %d", &value) > 0) {
            output += (double) value / depth;
        } else {
            break;
        }
    } while (!feof (stdin) && (depth > 1));

    printf ("%lg\n", output);
}
