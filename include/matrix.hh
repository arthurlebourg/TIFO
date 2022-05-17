#pragma once

#include <cmath>
#include <functional>
#include <numeric>
#include <vector>

#include "utils.hh"

template <typename T>
class Matrix
{
public:
    // rows == height
    // cols == width
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

    void set_values(std::vector<T> val);

    T get_min();

    T get_max();

    void apply(const std::function<T(T)> &func);

    Matrix<T> convolve(Matrix<T> kernel);

    size_t get_rows();

    size_t get_cols();

    std::vector<T> get_mData();

    T at(size_t x, size_t y);

    void set_value(size_t x, size_t y, T val);

    Matrix<T> operator+=(const Matrix<T> &rhs);

    Matrix<T> operator*=(const Matrix<T> &rhs);

    Matrix<T> operator*(const Matrix<T> &rhs);

    Matrix<T> operator/(const Matrix<T> &rhs);

    Matrix<T> operator-();

    Matrix<T> operator+(const Matrix<T> &rhs);

    Matrix<T> operator-(const Matrix<T> &rhs);

    bool isonboundary(size_t x, size_t y, size_t sz);

private:
    size_t mRows;
    size_t mCols;
    std::vector<T> mData;
};

#include "matrix.hxx"
