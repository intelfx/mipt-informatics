#include <stdio.h>
#include <stdlib.h>

struct Description
{
	bool is_male : 1;
	bool is_clever : 1;
	bool has_hat : 1;
	bool has_trousers_or_skirt : 1;

	enum : unsigned {
		HAIR_BLONDE = 0,
		HAIR_RED = 1,
		HAIR_BROWN = 2,
		HAIR_BLACK = 3
	} hair_color : 2;

	enum : unsigned {
		EYE_BLUE = 0,
		EYE_GREEN = 1,
		EYE_GRAY = 2,
		EYE_DARK = 3
	} eye_color : 2;

	const char* gender() const { return is_male ? "ma" : "fe"; }
	const char* cleverness() const { return is_clever ? "cl" : "du"; }
	const char* hat() const { return has_hat ? "ha" : "nh"; }
	const char* clothing() const { return has_trousers_or_skirt ? "tr" : "sk"; }
	const char* hair() const
	{
		switch (hair_color)
		{
		case HAIR_BLONDE: return "bn";
		case HAIR_RED: return "rd";
		case HAIR_BROWN: return "bw";
		case HAIR_BLACK: return "bk";
		default: abort();
		}
	}

	const char* eye() const
	{
		switch (eye_color)
		{
		case EYE_BLUE: return "bu";
		case EYE_GREEN: return "ge";
		case EYE_GRAY: return "gy";
		case EYE_DARK: return "da";
		default: abort();
		}
	}
} __attribute__((packed));

int main()
{
	Description d;
	scanf ("%hhx", (unsigned char*)&d);

	printf ("%s\n", d.hat());
}