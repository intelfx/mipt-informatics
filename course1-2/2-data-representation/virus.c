#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

void* create_bitset (size_t N)
{
	return calloc ((N + 7) / 8, 1);
}

void destroy_bitset (void* bitset)
{
	free (bitset);
}

void set_bitset (void* bitset, size_t N, unsigned bit)
{
	size_t byte_idx = N / 8;
	size_t bit_idx = N % 8;
	uint8_t* byte = (uint8_t*)bitset + byte_idx;

	if (bit) {
		*byte |= 1 << bit_idx;
	} else {
		*byte &= ~(1 << bit_idx);
	}
}

unsigned get_bitset (void* bitset, size_t N)
{
	size_t byte_idx = N / 8;
	size_t bit_idx = N % 8;
	uint8_t* byte = (uint8_t*)bitset + byte_idx;

	return (*byte >> bit_idx) & 1;
}

int main()
{
	size_t infected, count;
	scanf ("%zu %zu", &infected, &count);

	void* bitset = create_bitset (count);

	for (size_t i = 0; i < infected; ++i) {
		size_t nr;
		scanf ("%zu", &nr);

		size_t delta = nr;
		for (size_t j = nr; j < count; j += delta++) {
			set_bitset (bitset, j, 1);
		}
	}

	size_t infected_count = 0;

	for (size_t i = 0; i < count; ++i) {
		if (get_bitset (bitset, i)) {
			++infected_count;
		}
	}

	destroy_bitset (bitset);

	printf ("%zu\n", infected_count);
}