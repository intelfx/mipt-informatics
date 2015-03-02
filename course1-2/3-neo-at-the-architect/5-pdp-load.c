#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <stdint.h>
#include <inttypes.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint16_t adr;

byte mem[UINT16_MAX];

byte b_read(adr a)
{
	return mem[a];
}

void b_write(adr a, byte val)
{
	mem[a] = val;
}

word w_read(adr a)
{
	return le16toh(*(word *) (mem + a));
}

void w_write(adr a, word val)
{
	*(word *) (mem + a) = htole16(val);
}

void load_file(void)
{
	scanf(" ");
	while (!feof(stdin)) {
		adr offset;
		word size;
		scanf("%" SCNx16 "%" SCNx16, &offset, &size);

		for (word i = 0; i < size; ++i) {
			byte b;
			scanf("%" SCNx8, &b);
			b_write(offset + i, b);
		}

		scanf(" ");
	}
}

void mem_dump(adr start, word n)
{
	for (word i = 0; i < n; i += 2) {
		word w = w_read(start + i);
		printf("%06" PRIo16 " : " "%06" PRIo16 "\n", start + i, w);
	}
}
