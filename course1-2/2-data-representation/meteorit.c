#define _BSD_SOURCE
#include <endian.h>

union data convert (union data x)
{
	union data result;
	result.u64 = htobe64 (le64toh (x.u64));
	return result;
}