#include <iostream>
#include <string.h>

struct Matrix
{
    size_t rows, columns;
    double* data;

    Matrix (int r, int c)
        : rows (r)
        , columns (c)
        , data (new double[rows * columns])
    {
    }

    ~Matrix()
    {
        delete[] data;
    }

    Matrix (const Matrix& rhs)
        : rows (rhs.rows)
        , columns (rhs.columns)
        , data (new double[rows * columns])
    {
        memcpy (data, rhs.data, sizeof (*data) * rows * columns);
    }

    Matrix& operator= (const Matrix& rhs) = delete;

    friend std::ostream& operator<< (std::ostream& out, const Matrix& m)
    {
        for (size_t r = 0; r < m.rows; ++r) {
            for (size_t c = 0; c < m.columns - 1; ++c) {
                out << m.data[r * m.columns + c] << " ";
            }
            out << m.data[(r + 1) * m.columns - 1] << std::endl;
        }

        return out;
    }
};
