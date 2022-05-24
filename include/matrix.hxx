#include <algorithm>
#include <iostream>
#include <tbb/parallel_for.h>

#include "matrix.hh"

template <typename T>
void Matrix<T>::set_values(std::vector<T> &val)
{
    if (val.size() != mRows * mCols)
    {
        std::cerr << "Error: vector length != matrix size" << std::endl;
        return;
    }
    mData = val; // copy, equivalent to std::copy
}

template <typename T>
void Matrix<T>::fill(T val)
{
    mData.assign(mData.size(), val);
}

template <typename T>
void Matrix<T>::swap(Matrix<T> &mat)
{
    mData.swap(mat.get_data());
}

template <typename T>
T Matrix<T>::get_min()
{
    return *std::min_element(mData.begin(), mData.end());
}

template <typename T>
T Matrix<T>::get_max()
{
    return *std::max_element(mData.begin(), mData.end());
}

template <typename T>
std::pair<T, T> Matrix<T>::get_minmax()
{
    auto minmax = std::minmax_element(mData.begin(), mData.end());
    return { *minmax.first, *minmax.second };
}

template <typename T>
void Matrix<T>::apply(const std::function<T(T, size_t)> &func)
{
    tbb::parallel_for(tbb::blocked_range<size_t>(0, mRows * mCols),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              mData[i] = func(mData[i], i);
                          }
                      });
}

template <typename T>
void Matrix<T>::convolve(Matrix<T> &kernel, Matrix<T> &output)
{
    int kCenterX = kernel.mCols / 2;
    int kCenterY = kernel.mRows / 2;

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, mRows),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < mCols; j++)
                {
                    T acc{};

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
                                acc += mData[ii * mCols + jj]
                                    * kernel.mData[m * kernel.mCols + n];
                            }
                        }
                    }

                    output.mData[i * mCols + j] = acc;
                }
            }
        });
}

template <typename T>
void Matrix<T>::convolve(Matrix<T> &kernel, Matrix<T> &output, size_t padding)
{
    int kCenterX = kernel.mCols / 2;
    int kCenterY = kernel.mRows / 2;

    tbb::parallel_for(
        tbb::blocked_range<size_t>(padding, mRows - padding),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = padding; j < mCols - padding; j++)
                {
                    T acc{};

                    for (size_t m = 0; m < kernel.mRows; m++)
                    {
                        size_t mm = kernel.mRows - 1 - m;

                        for (size_t n = 0; n < kernel.mCols; n++)
                        {
                            size_t nn = kernel.mCols - 1 - n;

                            // index of input signal, used for checking boundary
                            size_t ii = i + (kCenterY - mm);
                            size_t jj = j + (kCenterX - nn);

                            acc += mData[ii * mCols + jj]
                                * kernel.mData[m * kernel.mCols + n];
                        }
                    }

                    output.mData[i * mCols + j] = acc;
                }
            }
        });
}

template <typename T>
void Matrix<T>::morph(Matrix<T> &kernel, bool is_dilation, Matrix<T> &output)
{
    size_t krows = kernel.get_rows();
    size_t kcols = kernel.get_cols();

    size_t sx = krows / 2 + krows % 2;
    size_t sy = kcols / 2 + kcols % 2;

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, mRows),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < mCols; j++)
                {
                    T acc{};
                    if (!is_on_boundary(j, i, sx, sy))
                    {
                        for (size_t ii = 0; ii < krows; ii++)
                        {
                            for (size_t jj = 0; jj < kcols; jj++)
                            {
                                if (kernel.get_value(jj, ii) < 0.5)
                                    continue;

                                auto val = get_value(j + jj - sx, i + ii - sy);

                                if (is_dilation && val > acc)
                                    acc = val;
                                else if (!is_dilation && val < acc)
                                    acc = val;
                            }
                        }
                    }
                    output.set_value(j, i, acc);
                }
            }
        });
}

template <typename T>
size_t Matrix<T>::get_rows()
{
    return mRows;
}

template <typename T>
size_t Matrix<T>::get_cols()
{
    return mCols;
}

template <typename T>
std::vector<T> &Matrix<T>::get_data()
{
    return mData;
}

template <typename T>
const std::vector<T> &Matrix<T>::get_data() const
{
    return mData;
}

template <typename T>
T Matrix<T>::get_value(size_t x, size_t y)
{
    return mData[y * mCols + x];
}

template <typename T>
void Matrix<T>::set_value(size_t x, size_t y, T val)
{
    mData[y * mCols + x] = val;
}

template <typename T>
T Matrix<T>::safe_at(size_t x, size_t y)
{
    return is_in_bound(x, y) ? mData[y * mCols + x] : T{};
}

template <typename T>
void Matrix<T>::safe_set(size_t x, size_t y, T val)
{
    if (is_in_bound(x, y))
        mData[y * mCols + x] = val;
}

template <typename T>
Matrix<T> Matrix<T>::operator+=(const Matrix<T> &rhs)
{
    if (mCols != rhs.mCols || rhs.mRows != mRows)
    {
        std::cerr << "Error: Matrix operator+=, matrices are not the same size"
                  << std::endl;
        return *this;
    }
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        mData[i] += rhs.mData[i];
    }
    return *this;
}

template <typename T>
Matrix<T> Matrix<T>::operator*=(const Matrix<T> &rhs)
{
    if (mCols != rhs.mCols || rhs.mRows != mRows)
    {
        std::cerr << "Error: Matrix operator*=, matrices are not the same size"
                  << std::endl;
        return *this;
    }
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        mData[i] *= rhs.mData[i];
    }
    return *this;
}

template <typename T>
Matrix<T> Matrix<T>::operator*(const Matrix<T> &rhs)
{
    Matrix<T> res = Matrix(mRows, mCols, 0);
    if (mCols != rhs.mCols || rhs.mRows != mRows)
    {
        std::cerr << "Error: Matrix operator*, matrices are not the same size"
                  << std::endl;
        return res;
    }
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        res.mData[i] = mData[i] * rhs.mData[i];
    }
    return res;
}

template <typename T>
Matrix<T> Matrix<T>::operator/(const Matrix<T> &rhs)
{
    Matrix<T> res = Matrix(mRows, mCols, 0);
    if (mCols != rhs.mCols || rhs.mRows != mRows)
    {
        std::cerr << "Error: Matrix operator/, matrices are not the same size"
                  << std::endl;
        return res;
    }
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        res.mData[i] = mData[i] / rhs.mData[i];
    }
    return res;
}

template <typename T>
Matrix<T> Matrix<T>::operator-()
{
    for (size_t i = 0; i < mCols * mRows; i++)
    {
        mData[i] *= -1;
    }
    return *this;
}

template <typename T>
Matrix<T> Matrix<T>::operator+(const Matrix<T> &rhs)
{
    Matrix<T> res = Matrix(mRows, mCols, 0);
    if (mCols != rhs.mCols || rhs.mRows != mRows)
    {
        std::cerr << "Error: Matrix operator+, matrices are not the same size"
                  << std::endl;
        return res;
    }
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        res.mData[i] = mData[i] + rhs.mData[i];
    }
    return res;
}

template <typename T>
Matrix<T> Matrix<T>::operator-(const Matrix<T> &rhs)
{
    Matrix<T> res = Matrix(mRows, mCols, 0);
    if (mCols != rhs.mCols || rhs.mRows != mRows)
    {
        std::cerr << "Error: Matrix operator-, matrices are not the same size"
                  << std::endl;
        return res;
    }
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        res.mData[i] = mData[i] - rhs.mData[i];
    }
    return res;
}

template <typename T>
bool Matrix<T>::is_in_bound(size_t x, size_t y)
{
    return x < mCols && y < mRows;
}

template <typename T>
bool Matrix<T>::is_on_boundary(size_t x, size_t y, size_t sx, size_t sy)
{
    return (!(x >= sx && x < (mCols - sx)) || !(y >= sy && y < (mRows - sy)));
}

template <typename T>
void Matrix<T>::pad_borders(size_t padding)
{
    for (size_t p = 0; p < padding; p++)
    {
        for (size_t i = padding; i < mRows - padding; i++)
        {
            // left
            set_value(padding - 1 - p, i, get_value(padding + p, i));
            // right
            set_value(mCols - padding + p, i,
                      get_value(mCols - 1 - padding - p, i));
        }
        for (size_t i = padding; i < mCols - padding; i++)
        {
            // top
            set_value(i, padding - 1 - p, get_value(i, padding + p));
            // bottom
            set_value(i, mRows - padding + p,
                      get_value(i, mRows - 1 - padding - p));
        }
    }

    auto top_left = get_value(padding, padding);
    auto top_right = get_value(mCols - 1 - padding, padding);
    auto bottom_left = get_value(padding, mRows - 1 - padding);
    auto bottom_right = get_value(mCols - 1 - padding, mRows - 1 - padding);

    for (size_t i = 0; i < padding; i++)
    {
        for (size_t j = 0; j < padding; j++)
        {
            set_value(j, i, top_left);
            set_value(mCols - 1 - padding + j, i, top_right);
            set_value(j, mRows - 1 - padding + i, bottom_left);
            set_value(mCols - 1 - padding + j, mRows - 1 - padding + i,
                      bottom_right);
        }
    }
}

template <typename T>
void Matrix<T>::to_padded(size_t padding, Matrix<T> &output)
{
    tbb::parallel_for(tbb::blocked_range<size_t>(0, mRows),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = 0; j < mCols; j++)
                              {
                                  output.set_value(j + padding, i + padding,
                                                   get_value(j, i));
                              }
                          }
                      });

    output.pad_borders(padding);
}

template <typename T>
void Matrix<T>::to_unpad(size_t padding, Matrix<T> &output)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, output.get_rows()),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < output.get_cols(); j++)
                {
                    output.set_value(j, i, get_value(j + padding, i + padding));
                }
            }
        });
}
