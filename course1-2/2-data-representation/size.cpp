#include <limits>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	int mantissa_size = 0;
	char type[30];

	scanf ("%[^\n]", type);
	if (!strcmp (type, "float")) {
		mantissa_size = std::numeric_limits<float>::digits;
	} else if (!strcmp (type, "double")) {
		mantissa_size = std::numeric_limits<double>::digits;
	} else if (!strcmp (type, "long double")) {
		mantissa_size = std::numeric_limits<long double>::digits;
	} else {
		abort();
	}

	printf ("%d\n", mantissa_size - 1);
}