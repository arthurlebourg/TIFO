#pragma once

#include <cmath>
#include <functional>
#include <numeric>
#include <vector>

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
    std::pair<T, T> get_minmax();

    void apply(const std::function<T(T, size_t)> &func);
    void convolve(Matrix<T> &kernel, Matrix<T> &output);
    void morph(Matrix<T> &kernel, bool is_dilation, Matrix<T> &output);

    size_t get_rows();
    size_t get_cols();

    std::vector<T> &get_data();
    const std::vector<T> &get_data() const;

    // No bounds check
    T get_value(size_t x, size_t y);
    void set_value(size_t x, size_t y, T val);

    // With bounds check
    T safe_at(size_t x, size_t y);

    Matrix<T> operator+=(const Matrix<T> &rhs);
    Matrix<T> operator*=(const Matrix<T> &rhs);
    Matrix<T> operator*(const Matrix<T> &rhs);
    Matrix<T> operator/(const Matrix<T> &rhs);
    Matrix<T> operator-();
    Matrix<T> operator+(const Matrix<T> &rhs);
    Matrix<T> operator-(const Matrix<T> &rhs);

    bool is_in_bound(size_t x, size_t y);

    bool is_on_boundary(size_t x, size_t y, size_t sx, size_t sy);

private:
    size_t mRows;
    size_t mCols;
    std::vector<T> mData;
};

#include "matrix.hxx"
