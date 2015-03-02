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
	size_t N;
	scanf ("%zu", &N);

	void* bitset = create_bitset (N + 1);

	set_bitset (bitset, 0, 1);
	set_bitset (bitset, 1, 1);

	for (size_t i = 2; i*i <= N; ++i) {
		if (!get_bitset (bitset, i)) {
			for (size_t j = i*i; j <= N; j += i) {
				set_bitset (bitset, j, 1);
			}
		}
	}

	size_t prime_count = 0;

	for (size_t i = 0; i <= N; ++i) {
		if (!get_bitset (bitset, i)) {
			++prime_count;
		}
	}

	destroy_bitset (bitset);

	printf ("%zu\n", prime_count);
}