#define _BSD_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <endian.h>
#include <stdint.h>
#include <stdlib.h>

char signature[] =
{
	0x75, 0x11
};

char replace[] =
{
	0x90, 0x90
};

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

	void* to_patch = memmem (buf, st.st_size, signature, sizeof (signature));
	if (!to_patch)
		return 1;

	memcpy (to_patch, replace, sizeof (replace));

	FILE* cracked = fopen ("cracked", "wb");
	assert (cracked && !ferror (cracked));
	cnt = fwrite (buf, 1, st.st_size, cracked);
	if (cnt != st.st_size)
		return 1;
	fclose (cracked);

	free (buf);
	return 0;
}