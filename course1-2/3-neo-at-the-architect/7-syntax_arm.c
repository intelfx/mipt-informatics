#define _POSIX_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// #define PARSER_DEBUG

/*
 * The motto of this solution: "Doing what I have with what I got".
 */

struct Vector {
	void **data;
	size_t length, allocated;
};

struct Vector vector_new(void)
{
	struct Vector vec;
	memset(&vec, 0, sizeof(vec));
	return vec;
}

void **vector_add(struct Vector *vec, void *ptr)
{
	if (vec->length >= vec->allocated) {
		if (vec->allocated) {
			assert(vec->data);
			void *new_data = realloc(vec->data,
						 sizeof(ptr) * vec->allocated *
						 2);
			assert(new_data);
			vec->allocated *= 2;
			vec->data = new_data;
		} else {
			assert(!vec->data);
			vec->data = malloc(sizeof(ptr));
			vec->allocated = 1;
			assert(vec->data);
		}
	}

	void **pptr = vec->data + vec->length++;
	*pptr = ptr;
	return pptr;
}

void *vector_remove(struct Vector *vec)
{
	assert(vec->length);
	return vec->data[--vec->length];
}

void vector_delete(struct Vector *vec)
{
	free(vec->data);
}

#define VECTOR_FOREACH(vec, type, var) for (type **iter = (type**)((vec)->data), **end = iter + (vec)->length, *var = *iter; iter != end; var = *++iter)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef int bool;
static const bool true = 1, false = 0;

static const size_t READ_BLOCK_SIZE = 4096;
static const char *delimiters = " \t\r\n,()[]";

char *data = 0;
size_t data_allocd = 0;
size_t data_len = 0;

struct Vector words;

enum {
	ISA_X86 = 0,
	ISA_ARM
} instruction_set_architecture = ISA_X86;

enum {
	ARM_VFP_ABSENT = 0,
	ARM_VFP_PRESENT
} arm_vfp_insns = ARM_VFP_ABSENT;

enum {
	X86_FLAVOR_INTEL = 0,
	X86_FLAVOR_ATT
} x86_flavor = X86_FLAVOR_INTEL;

enum {
	X87_ABSENT = 0,
	X87_PRESENT
} x87_insns = X87_ABSENT;

enum {
	X86_OPERAND_16 = 0,
	X86_OPERAND_32,
	X86_OPERAND_64
} x86_operands = X86_OPERAND_16;

#ifdef PARSER_DEBUG
const char *strings_ISA[] = { "x86", "ARM" };
const char *strings_ARM_VFP[] = { "VFP absent", "VFP present" };
const char *strings_x86_flavor[] = { "Intel", "AT&T" };
const char *strings_x86_FPU[] = { "FPU absent", "FPU present" };
const char *strings_x86_operand_size[] = { "word", "dword", "qword" };
#endif				// PARSER_DEBUG

void data_realloc(size_t size);
void data_append(const char *str);

void cut_string_literals();
void lowercase();
void tokenize();

bool check_arm_and_vfp_insns();
void check_x86_regs_flavor_and_size();
void check_x87_commands();

int main()
{
	words = vector_new();

	// Read the whole program into the buffer.
	while (!(feof(stdin) || ferror(stdin))) {
		// Reallocate input buffer if necessary.
		data_realloc(data_len + READ_BLOCK_SIZE);

		// Read data.
		data_len += fread(data + data_len, 1, READ_BLOCK_SIZE, stdin);
	}

	data_realloc(data_len + 1);
	data[data_len++] = '\0';

	// Remove string literals from the input
	cut_string_literals();

	// Transform the input to lowercase
	lowercase();

#ifdef PARSER_DEBUG
	fwrite(data, 1, data_len, stderr);
	putchar('\n');
#endif

	// Split the input in words
	tokenize();

	/*
	 * I could show off with actually knowing both x86 and ARM instruction sets,
	 * but let's just do what's asked.
	 */

	if (check_arm_and_vfp_insns()) {
		// no-op
	} else {
		check_x86_regs_flavor_and_size();
		check_x87_commands();
	}

#ifdef PARSER_DEBUG
	fprintf(stderr, "ISA: %s\n", strings_ISA[instruction_set_architecture]);

	switch (instruction_set_architecture) {
	case ISA_ARM:
		fprintf(stderr, "ARM VFP: %s\n",
			strings_ARM_VFP[arm_vfp_insns]);
		break;

	case ISA_X86:
		fprintf(stderr, "x86 flavor: %s\n",
			strings_x86_flavor[x86_flavor]);
		fprintf(stderr, "x86 max. operand size: %s\n",
			strings_x86_operand_size[x86_operands]);
		fprintf(stderr, "x86 FPU: %s\n", strings_x86_FPU[x87_insns]);
		break;

	default:
		abort();
	}
#else
	if (instruction_set_architecture == ISA_X86) {
		printf("86 ");

		printf("%d ", x86_flavor == X86_FLAVOR_ATT);
		printf("%d ", x87_insns == X87_PRESENT);

		switch (x86_operands) {
		case X86_OPERAND_64:
			printf("64");
			break;
		case X86_OPERAND_32:
			printf("32");
			break;
		case X86_OPERAND_16:
			printf("16");
			break;
		}
	} else {
		printf("0 ");

		printf("%d", arm_vfp_insns == ARM_VFP_PRESENT);
	}

	putchar('\n');
#endif				// PARSER_DEBUG

	free(data);
	vector_delete(&words);

	return false;
}

void data_realloc(size_t size)
{
	if (size > data_allocd) {
		char *new_data = (char *)realloc(data, size);
		if (!new_data) {
			abort();
		}
		data = new_data;
		data_allocd = size;
	}
}

void data_append(const char *str)
{
	size_t str_len = strlen(str);

	// Reallocate input buffer if necessary.
	data_realloc(data_len + str_len);

	// Append string.
	memcpy(data + data_len, str, str_len);
	data_len += str_len;
}

void cut_string_literals()
{
	char *pos = data, *begin = 0, *end = 0, *quote;

	while ((quote = strchr(pos, '"'))) {
		if (begin) {
			end = quote;
			memmove(begin, end + 1, data_len - (end + 1 - data));
			data_len -= end + 1 - begin;
			pos = begin;
			begin = 0;
			end = 0;
		} else {
			begin = quote;
			pos = begin + 1;
		}
	}
}

void lowercase()
{
	for (char *pos = data; pos != data + data_len; ++pos) {
		*pos = tolower(*pos);
	}
}

void tokenize()
{
	char *strtok_session, *strtok_ptr = data, *token;

	while ((token = strtok_r(strtok_ptr, delimiters, &strtok_session))) {
		strtok_ptr = 0;
		vector_add(&words, token);
	}
}

bool check_arm_and_vfp_insns()
{
	VECTOR_FOREACH(&words, char, word) {
		if (!strcmp(word, "vldr") || !strcmp(word, "vstr")) {
			instruction_set_architecture = ISA_ARM;
			arm_vfp_insns = ARM_VFP_PRESENT;
			// nothing more to search for
			return true;
		} else if (!strcmp(word, "ldr") || !strcmp(word, "str")) {
			instruction_set_architecture = ISA_ARM;
			// keep searching for vfp insns
		}
	}

	return (instruction_set_architecture == ISA_ARM);
}

bool check_x86_gp_reg(const char *word)
{
	if (!word[2]) {
		switch (word[1]) {
		case 'x':
			return (word[0] >= 'a' && word[0] <= 'd');
		case 'i':
			return (word[0] == 's' || word[0] == 'd');
		case 'p':
			return (word[0] == 's' || word[0] == 'b');
		default:
			return false;
		}
	}

	return false;
}

bool check_x86_reg(const char *word)
{
	// segment registers; cs, ds, ss
	if (word[1] == 's' && !word[2]) {
		switch (word[0]) {
		case 'c':
		case 'd':
		case 's':
			return true;
		default:
			return false;
		}
	}

	if (check_x86_gp_reg(word)) {
		return true;
	}

	if (check_x86_gp_reg(word + 1)) {
		switch (word[0]) {
		case 'e':
			x86_operands = MAX(x86_operands, X86_OPERAND_32);
			return true;
		case 'r':
			x86_operands = MAX(x86_operands, X86_OPERAND_64);
			return true;
		default:
			return false;
		}
	}

	return false;
}

void check_x86_regs_flavor_and_size()
{
	VECTOR_FOREACH(&words, char, word) {
		if (word[0] == '%' && check_x86_reg(word + 1)) {
			x86_flavor = X86_FLAVOR_ATT;
			if (x86_operands == X86_OPERAND_64) {
				// nothing more to search for
				return;
			}
		} else {
			check_x86_reg(word);
		}
	}
}

void check_x87_commands()
{
	VECTOR_FOREACH(&words, char, word) {
		if (!strcmp(word, "finit") || !strcmp(word, "fld")
		    || !strcmp(word, "fst")) {
			x87_insns = X87_PRESENT;
			// nothing more to search for
			return;
		}
	}
}
