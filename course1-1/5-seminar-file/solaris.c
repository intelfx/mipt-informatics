#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <endian.h>

int main()
{
    FILE *src = fopen ("solaris.dat", "r"),
         *dst = fopen ("solaris.ans", "w");
    uint16_t buf;

    while (fread (&buf, sizeof (buf), 1, src) == 1) {
        buf = htobe16 (le16toh (buf));
        fwrite (&buf, sizeof (buf), 1, dst);
    }

    fclose (src);
    fclose (dst);
}
