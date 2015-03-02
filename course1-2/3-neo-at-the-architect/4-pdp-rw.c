#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <stdint.h>

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
