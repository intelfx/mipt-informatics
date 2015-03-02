#include <stdio.h>

int main()
{
	char c;
	scanf ("%hhd", &c);

	for (unsigned i = 0; i < 8; ++i) {
		printf ("%d", (c >> (7 - i)) & 1);
	}

	putchar ('\n');
}