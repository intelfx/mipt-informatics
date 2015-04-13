#define _BSD_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <endian.h>
#include <stdint.h>
#include <stdlib.h>

static const uint32_t time_source = htole32 (1000000000),
                      time_cracked = htole32 (2006632960);

int main()
{
	struct stat st;
	int r = stat ("source", &st);
	if (r < 0)
		return 1;

	char* buf = malloc (st.st_size);

	FILE* source = fopen ("source", "rb");
	assert (source && !ferror (source));
	size_t cnt = fread (buf, 1, st.st_size, source);
	if (cnt != st.st_size)
		return 1;
	fclose (source);

	void* to_patch = memmem (buf, st.st_size, &time_source, sizeof (time_source));
	if (!to_patch)
		return 1;

	*(uint32_t*)to_patch = time_cracked;

	FILE* cracked = fopen ("cracked", "wb");
	assert (cracked && !ferror (cracked));
	cnt = fwrite (buf, 1, st.st_size, cracked);
	if (cnt != st.st_size)
		return 1;
	fclose (cracked);

	free (buf);
	return 0;
}