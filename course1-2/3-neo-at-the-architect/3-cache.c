#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

unsigned int gcd(unsigned int u, unsigned int v)
{
	int shift;

	/* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
	if (u == 0)
		return v;
	if (v == 0)
		return u;

	/* Let shift := lg K, where K is the greatest power of 2
	   dividing both u and v. */
	for (shift = 0; ((u | v) & 1) == 0; ++shift) {
		u >>= 1;
		v >>= 1;
	}

	while ((u & 1) == 0)
		u >>= 1;

	/* From here on, u is always odd. */
	do {
		/* remove all factors of 2 in v -- they are not common */
		/*   note: v is not zero, so while will terminate */
		while ((v & 1) == 0)	/* Loop X */
			v >>= 1;

		/* Now u and v are both odd. Swap if necessary so u <= v,
		   then set v = v - u (which is even). For bignums, the
		   swapping is just pointer movement, and the subtraction
		   can be done in-place. */
		if (u > v) {
			unsigned int t = v;
			v = u;
			u = t;
		}		// Swap u and v.
		v = v - u;	// Here v >= u.
	} while (v != 0);

	/* restore common factors of 2 */
	return u << shift;
}

void print_matrix(const char *name, unsigned *array, size_t N)
{
	printf("%s:\n", name);
	for (size_t y = 0; y < N; ++y) {
		for (size_t x = 0; x < N; ++x) {
			printf("%4u ", array[y * N + x]);
		}
		putchar('\n');
	}
	putchar('\n');
}

void transpose_in_place(unsigned *array, size_t N)
{
	for (size_t y = 0; y < N; ++y) {
		for (size_t x = y + 1; x < N; ++x) {
			unsigned *ptr_a = &array[y * N + x],
			    *ptr_b = &array[x * N + y];
			unsigned tmp = *ptr_a;
			*ptr_a = *ptr_b;
			*ptr_b = tmp;
		}
	}
}

void multiply_matrices_transposed(unsigned *AB, unsigned *A, unsigned *B,
				  size_t Ax, size_t Ay, size_t Bx, size_t By)
{
	assert(Ay == Bx);

	unsigned *AB_cell = AB;

	for (size_t ABy = 0; ABy < By; ++ABy) {
		for (size_t ABx = 0; ABx < Ax; ++ABx) {
			unsigned *A_row = &A[ABy * Ax];
			unsigned *B_column = &B[ABx * By];	/* B is transposed */
			*AB_cell = 0;
			for (size_t i = 0; i < Ay; ++i) {
				*AB_cell += *A_row++ * *B_column++;
			}
			++AB_cell;
		}
	}
}

int main()
{
	size_t N;
	unsigned divisor;
	unsigned *A, *B, *AB;
	scanf("%zu %u", &N, &divisor);

	A = malloc(N * N * sizeof(*A));
	B = malloc(N * N * sizeof(*B));
	AB = malloc(N * N * sizeof(*AB));

	for (size_t i = 0, x = 0, y = 0; y < N; ++i, ++x) {
		if (x >= N) {
			++y;
			x = 0;
		}
		A[i] = gcd(x + 1, y + 1);
	}

	for (size_t i = 0, x = 0, y = 0; y < N; ++i, ++x) {
		if (x >= N) {
			++y;
			x = 0;
		}
		B[i] = gcd(N - x + 1, N - y + 1);
	}

	/* transpose_in_place (B, N); */
	/* B, as well as A, is symmetrical wrt. transposition */

	multiply_matrices_transposed(AB, A, B, N, N, N, N);

	/* transpose_in_place (AB, N); */
	/* this is not needed */

	/*
	   print_matrix ("A", A, N);
	   print_matrix ("B", B, N);
	   print_matrix ("AB", AB, N);
	 */

	size_t result = 0;
	for (size_t i = 0; i < N * N; ++i) {
		if (!(AB[i] % divisor)) {
			++result;
		}
	}

	printf("%zu\n", result);
}
