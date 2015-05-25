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
	0x04, 0xd1 // bne.n
};

char replace[] =
{
	0x04, 0xd0 // be.n
};

int main()
{
	struct stat st;
	int r = stat ("arm.elf", &st);
	if (r < 0)
		return 1;

	char* buf = malloc (st.st_size);

	FILE* source = fopen ("arm.elf", "rb");
	assert (source && !ferror (source));
	size_t cnt = fread (buf, 1, st.st_size, source);
	if (cnt != st.st_size)
		return 1;
	fclose (source);

	void* to_patch = memmem (buf, st.st_size, signature, sizeof (signature));
	if (!to_patch)
		return 1;

	memcpy (to_patch, replace, sizeof (replace));

	FILE* cracked = fopen ("patched", "wb");
	assert (cracked && !ferror (cracked));
	cnt = fwrite (buf, 1, st.st_size, cracked);
	if (cnt != st.st_size)
		return 1;
	fclose (cracked);

	free (buf);
	return 0;
}
