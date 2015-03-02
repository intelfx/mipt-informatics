#include <stdio.h>

void value_to_rwx (unsigned value)
{
	if (value & 4) {
		putchar ('r');
	} else {
		putchar ('-');
	}

	if (value & 2) {
		putchar ('w');
	} else {
		putchar ('-');
	}

	if (value & 1) {
		putchar ('x');
	} else {
		putchar ('-');
	}

}

int main()
{
	for (int i = 0; i < 3; ++i) {
		char c;
		scanf (" %c", &c);
		value_to_rwx (c - '0');
	}
	putchar ('\n');
}