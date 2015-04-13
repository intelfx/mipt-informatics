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
	0xd9, 0x45, 0x08, // fld dword [ebp+8]
	0xd8, 0xc1 // fadd st, st1
};

char replace[] =
{
	0xd9, 0xeb, // fldpi
	0xde, 0xc9, // fmulp
	0x90
};

static const size_t ALLOC_STEP = 1024;

int main()
{
	char* buf = malloc (ALLOC_STEP);
	size_t buf_size = ALLOC_STEP;
	size_t offset = 0;

	for (;;) {
		size_t cnt = fread (buf + offset, 1, ALLOC_STEP, stdin);
		offset += cnt;
		if (ferror (stdin))
			return 1;
		if (feof (stdin) || cnt != ALLOC_STEP)
			break;

		buf_size += ALLOC_STEP;
		buf = realloc (buf, buf_size);
		if (!buf)
			return 1;
	}

	void* to_patch = memmem (buf, offset, signature, sizeof (signature));
	if (!to_patch)
		return 1;

	memcpy (to_patch, replace, sizeof (replace));

	size_t cnt = fwrite (buf, 1, offset, stdout);
	if (cnt != offset)
		return 1;

	free (buf);
	return 0;
}