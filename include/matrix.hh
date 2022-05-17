#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

#include "utils.hh"

template <typename T>
class Matrix
{
public:
    Matrix(size_t rows, size_t cols)
        : mRows(rows)
        , mCols(cols)
        , mData(rows * cols)
    {}

    Matrix(size_t rows, size_t cols, T val)
        : mRows(rows)
        , mCols(cols)
        , mData(rows * cols, val)
    {}

    Matrix(size_t rows, size_t cols, std::vector<T> mData)
        : mRows(rows)
        , mCols(cols)
        , mData(mData)
    {}

    void set_values(std::vector<T> val)
    {
        if (val.size() != mRows * mCols)
        {
            std::cerr << "Error: vector length != matrix size" << std::endl;
            return;
        }
        size_t index = 0;
        for (size_t i = 0; i < mRows; i++)
        {
            for (size_t j = 0; j < mCols; j++)
            {
                mData[i * mCols + j] = val[index++];
            }
        }
    }

    T get_min()
    {
        T min = 0;
        for (size_t i = 0; i < mRows * mCols; i++)
        {
            if (mData[i] < min)
                min = mData[i];
        }
        return min;
    }

    T get_max()
    {
        T max = 0;
        for (size_t i = 0; i < mRows * mCols; i++)
        {
            if (mData[i] > max)
                max = mData[i];
        }
        return max;
    }

    void apply(const std::function<T(T)> &func)
    {
        for (size_t y = 0; y < mRows; y++)
        {
            for (size_t x = 0; x < mCols; x++)
            {
                mData[y * mCols + x] = func(mData[y * mCols + x]);
            }
        }
    }

    Matrix<T> convolve(Matrix<T> kernel)
    {
        Matrix<T> convoluted = Matrix<T>(mRows, mCols, 0);
        int kCenterX = kernel.mCols / 2;
        int kCenterY = kernel.mRows / 2;

        for (size_t i = 0; i < mRows; i++)
        {
            for (size_t j = 0; j < mCols; j++)
            {
                for (size_t m = 0; m < kernel.mRows; m++)
                {
                    size_t mm = kernel.mRows - 1 - m;

                    for (size_t n = 0; n < kernel.mCols; n++)
                    {
                        size_t nn = kernel.mCols - 1 - n;

                        // index of input signal, used for checking boundary
                        size_t ii = i + (kCenterY - mm);
                        size_t jj = j + (kCenterX - nn);

                        // ignore input samples which are out of bound
                        if (ii < mRows && jj < mCols)
                        {
                            convoluted.mData[i * mCols + j] +=
                                mData[ii * mCols + jj]
                                * kernel.mData[m * kernel.mCols + n];
                        }
                    }
                }
            }
        }
        return convoluted;
    }

    size_t get_rows()
    {
        return mRows;
    }

    size_t get_cols()
    {
        return mCols;
    }

    std::vector<T> get_mData()
    {
        return mData;
    }

    T at(size_t x, size_t y)
    {
        return mData[y * mCols + x];
    }

    void set_value(size_t x, size_t y, T val)
    {
        mData[y * mCols + x] = val;
    }

    Matrix<T> operator+=(const Matrix<T> &rhs)
    {
        if (mCols != rhs.mCols || rhs.mRows != mRows)
        {
            std::cerr
                << "Error: Matrix operator+=, matrices are not the same size"
                << std::endl;
            return *this;
        }
        for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
        {
            mData[i] += rhs.mData[i];
        }
        return *this;
    }

    Matrix<T> operator*=(const Matrix<T> &rhs)
    {
        if (mCols != rhs.mCols || rhs.mRows != mRows)
        {
            std::cerr
                << "Error: Matrix operator*=, matrices are not the same size"
                << std::endl;
            return *this;
        }
        for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
        {
            mData[i] *= rhs.mData[i];
        }
        return *this;
    }

    Matrix<T> operator*(const Matrix<T> &rhs)
    {
        Matrix<T> res = Matrix(mRows, mCols, 0);
        if (mCols != rhs.mCols || rhs.mRows != mRows)
        {
            std::cerr
                << "Error: Matrix operator*, matrices are not the same size"
                << std::endl;
            return res;
        }
        std::vector<T> new_vec;
        for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
        {
            new_vec.push_back(mData[i] * rhs.mData[i]);
        }
        res.set_values(new_vec);
        return res;
    }

    Matrix<T> operator/(const Matrix<T> &rhs)
    {
        Matrix<T> res = Matrix(mRows, mCols, 0);
        if (mCols != rhs.mCols || rhs.mRows != mRows)
        {
            std::cerr
                << "Error: Matrix operator/, matrices are not the same size"
                << std::endl;
            return res;
        }
        std::vector<T> new_vec;
        for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
        {
            new_vec.push_back(mData[i] / rhs.mData[i]);
        }
        res.set_values(new_vec);
        return res;
    }

    Matrix<T> operator-()
    {
        for (size_t i = 0; i < mCols * mRows; i++)
        {
            mData[i] *= -1;
        }
        return *this;
    }

    Matrix<T> operator+(const Matrix<T> &rhs)
    {
        Matrix<T> res = Matrix(mRows, mCols, 0);
        if (mCols != rhs.mCols || rhs.mRows != mRows)
        {
            std::cerr
                << "Error: Matrix operator+, matrices are not the same size"
                << std::endl;
            return res;
        }
        std::vector<T> new_vec;
        for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
        {
            new_vec.push_back(mData[i] + rhs.mData[i]);
        }
        res.set_values(new_vec);
        return res;
    }

    Matrix<T> operator-(const Matrix<T> &rhs)
    {
        Matrix<T> res = Matrix(mRows, mCols, 0);
        if (mCols != rhs.mCols || rhs.mRows != mRows)
        {
            std::cerr
                << "Error: Matrix operator-, matrices are not the same size"
                << std::endl;
            return res;
        }
        std::vector<T> new_vec;
        for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
        {
            new_vec.push_back(mData[i] - rhs.mData[i]);
        }
        res.set_values(new_vec);
        return res;
    }

    static Matrix<float> ellipse_kernel(int height, int width)
    {
        int i, j;

        float inv_r2 = 0;
        int r = height / 2;
        int c = width / 2;
        inv_r2 = r ? 1. / ((float)r * r) : 0;

        std::vector<float> vec(height * width, 0);

        for (i = 0; i < height; i++)
        {
            int j1 = 0;
            int j2 = 0;

            int dy = i - r;
            if (std::abs(dy) <= r)
            {
                float tmp = c * std::sqrt((r * r - dy * dy) * inv_r2);

                int dx = saturate_cast<int, float>(
                    tmp, std::numeric_limits<int>::min(),
                    std::numeric_limits<int>::max());
                j1 = std::max(c - dx, 0);
                j2 = std::min(c + dx + 1, width);
            }

            for (j = 0; j < j1; j++)
                vec[i * width + j] = 0;
            for (; j < j2; j++)
                vec[i * width + j] = 1;
            for (; j < width; j++)
                vec[i * width + j] = 0;
        }

        return Matrix<float>(height, width, vec);
    }

    bool isonboundary(size_t x, size_t y, size_t sz)
    {
        return (!(x < (mRows - sz) && x >= sz)
                || !(y < (mCols - sz) && y >= sz));
    }

    friend std::ostream &operator<<(std::ostream &os, const Matrix<char> mat);

    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Matrix<U> mat);

private:
    size_t mRows;
    size_t mCols;
    std::vector<T> mData;
};

std::ostream &operator<<(std::ostream &os, const Matrix<char> mat)
{
    for (size_t i = 0; i < mat.mRows; i++)
    {
        for (size_t j = 0; j < mat.mCols; j++)
        {
            os << (int)mat.mData[i * mat.mCols + j] << " ";
        }
        os << std::endl;
    }
    return os;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Matrix<T> mat)
{
    for (size_t i = 0; i < mat.mRows; i++)
    {
        for (size_t j = 0; j < mat.mCols; j++)
        {
            os << mat.mData[i * mat.mCols + j] << " ";
        }
        os << std::endl;
    }
    return os;
}
