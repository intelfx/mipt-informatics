#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct Description
{
	bool is_male : 1;
	bool is_clever : 1;
	bool has_hat : 1;
	bool has_trousers_or_skirt : 1;

	enum HairColor : unsigned {
		HAIR_BLONDE = 0,
		HAIR_RED = 1,
		HAIR_BROWN = 2,
		HAIR_BLACK = 3
	} hair_color : 2;

	enum EyeColor : unsigned {
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

	void parse (const char* str)
	{
#define CHECK_STR(string, action) if (!strcmp (str, string)) { action; return; }
		CHECK_STR ("ma", is_male = 1)
		CHECK_STR ("fe", is_male = 0)
		CHECK_STR ("cl", is_clever = 1)
		CHECK_STR ("du", is_clever = 0)
		CHECK_STR ("ha", has_hat = 1)
		CHECK_STR ("nh", has_hat = 0)
		CHECK_STR ("tr", has_trousers_or_skirt = 1)
		CHECK_STR ("sk", has_trousers_or_skirt = 0)
		CHECK_STR ("bn", hair_color = HAIR_BLONDE)
		CHECK_STR ("rd", hair_color = HAIR_RED)
		CHECK_STR ("bw", hair_color = HAIR_BROWN)
		CHECK_STR ("bk", hair_color = HAIR_BLACK)
		CHECK_STR ("bu", eye_color = EYE_BLUE)
		CHECK_STR ("ge", eye_color = EYE_GREEN)
		CHECK_STR ("gy", eye_color = EYE_GRAY)
		CHECK_STR ("da", eye_color = EYE_DARK)
#undef CHECK_STR
	}
} __attribute__((packed));

int main()
{
	Description d;
	*(char*)&d = 0;

	char string[3];

	while (scanf (" %2s", string) == 1) {
		d.parse (string);
	}

	printf ("%hhx\n", *(char*)&d);
}