#include <stdio.h>
#include <string.h>
#include <stdint.h>

void write_by_half_octet (uint8_t* memory, size_t size)
{
	for (size_t i = size; i--; )
	{
		uint8_t byte = memory[i];

		for (size_t j = 8; j-- > 4; )
		{
			printf ("%d", (byte & (1 << j)) ? 1 : 0);
		}

		putchar (' ');

		for (size_t j = 4; j--; )
		{
			printf ("%d", (byte & (1 << j)) ? 1 : 0);
		}

		putchar (" \n"[!i]);
	}
}

int main()
{
	char tag[10];

	scanf ("%s", tag);

	union
	{
		uint8_t data[sizeof (long double)];
		float v_f;
		double v_d;
		long double v_ld;
	} value = { };

	if (!strcmp (tag, "float"))
	{
		scanf ("%f", &value.v_f);
		write_by_half_octet (value.data, sizeof (value.v_f));
	}

	else if (!strcmp (tag, "double"))
	{
		scanf ("%lf", &value.v_d);
		write_by_half_octet (value.data, sizeof (value.v_d));
	}

	else if (!strcmp (tag, "long"))
	{
		scanf ("%*s"); /* skip double */
		scanf ("%Lf", &value.v_ld);
		write_by_half_octet (value.data, sizeof (value.v_ld));
	}
}