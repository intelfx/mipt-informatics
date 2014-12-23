#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef long data_t;

struct Matrix
{
        size_t rows, columns;
        data_t *data;
};

#define DATA(m, r, c) ((m)->data[(r) * (m)->columns + (c)])

struct Matrix matrix_create(size_t rows, size_t columns)
{
        struct Matrix r;
        r.rows = rows;
        r.columns = columns;
        r.data = malloc(sizeof(data_t) * rows * columns);
        return r;
}

struct Matrix matrix_null(void)
{
        struct Matrix r = { };
        return r;
}

int matrix_is_null(const struct Matrix *m)
{
        return (m->data == NULL);
}

void matrix_destroy(struct Matrix *m)
{
        free(m->data);
}

void matrix_print(const struct Matrix *m)
{
        size_t i, j;
        for (i = 0; i < m->rows; ++i) {
                for (j = 0; j < m->columns; ++j) {
                        printf(" %4ld ", DATA(m, i, j));
                }
                putchar('\n');
        }
}

void matrix_read(struct Matrix *m)
{
        size_t i;
        for (i = 0; i < m->rows * m->columns; ++i) {
                scanf ("%ld", m->data + i);
        }
}

struct Matrix matrix_multiply(const struct Matrix *a, const struct Matrix *b)
{
        if (a->columns != b->rows) {
                return matrix_null();
        }

        struct Matrix r = matrix_create(a->rows, b->columns);

        size_t i, j, k;
        for (i = 0; i < r.rows; ++i) {
                for (j = 0; j < r.columns; ++j) {
                        DATA(&r, i, j) = 0;
                        for (k = 0; k < a->columns; ++k) {
                                DATA(&r, i, j) += DATA(a, i, k) * DATA(b, k, j);
                        }
                }
        }

        return r;
}

int main()
{
        size_t a_r, a_c, b_r, b_c;
        scanf ("%zu %zu %zu %zu", &a_r, &a_c, &b_r, &b_c);

        struct Matrix a = matrix_create(a_r, a_c),
                      b = matrix_create(b_r, b_c);
        matrix_read(&a);
        matrix_read(&b);

        struct Matrix r = matrix_multiply(&a, &b);
        if (matrix_is_null(&r)) {
                printf ("-1\n");
        } else {
                matrix_print(&r);
        }

        matrix_destroy(&a);
        matrix_destroy(&b);
        matrix_destroy(&r);
};
