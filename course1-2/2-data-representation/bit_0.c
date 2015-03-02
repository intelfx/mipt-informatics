#include <stdio.h>

int main()
{
	unsigned a, b;
	scanf ("%x %x", &a, &b);
	printf ("%02x\n", a ^ b);
}