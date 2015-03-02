#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const char* translation_table[16] = {
        "0000",
        "0001",
        "0010",
        "0011",
        "0100",
        "0101",
        "0110",
        "0111",
        "1000",
        "1001",
        "1010",
        "1011",
        "1100",
        "1101",
        "1110",
        "1111"
};

int main()
{
        char *value_hex = 0, *value_bin, *value_bin_ptr;
        size_t value_hex_length;

        scanf ("%ms", &value_hex);
        value_hex_length = strlen (value_hex);

        value_bin = malloc (value_hex_length * 4 + 1);
        value_bin_ptr = value_bin;

        for (size_t i = 0; i < value_hex_length; ++i) {
                char c = tolower (value_hex[i]);

                if (c >= '0' && c <= '9') {
                        value_bin_ptr = stpcpy (value_bin_ptr, translation_table[c - '0']);
                } else if (c >= 'a' && c <= 'f') {
                        value_bin_ptr = stpcpy (value_bin_ptr, translation_table[c - 'a' + 0xA]);
                } else {
                        fprintf (stderr, "offending character: '%c'\n", c);
                        abort();
                }
        }

        value_bin_ptr = value_bin;

        while (*value_bin_ptr == '0' && value_bin_ptr[1]) {
                ++value_bin_ptr;
        }

        printf ("%s\n", value_bin_ptr);

        free (value_hex);
        free (value_bin);

        return 0;
}