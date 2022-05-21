#include <algorithm>
#include <tbb/parallel_for.h>

#include "matrix.hh"

template <typename T>
void Matrix<T>::set_values(std::vector<T> val)
{
    if (val.size() != mRows * mCols)
    {
        std::cerr << "Error: vector length != matrix size" << std::endl;
        return;
    }
    mData = val; // copy, equivalent to std::copy
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
        tbb::blocked_range<size_t>(0, output.get_rows()),
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
void Matrix<T>::morph(Matrix<T> &kernel, bool is_dilation, Matrix<T> &output)
{
    float val;
    size_t sz = (kernel.get_rows() - 1) / 2;

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, output.get_rows()),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < mCols; j++)
                {
                    if (isonboundary(i, j, sz))
                    {
                        val = 0;
                    }
                    else
                    {
                        std::vector<float> list;
                        for (size_t ii = 0; ii < kernel.get_rows(); ii++)
                        {
                            for (size_t jj = 0; jj < kernel.get_cols(); jj++)
                            {
                                if (kernel.safe_at(jj, ii) == 1)
                                {
                                    list.push_back(
                                        safe_at(j + jj - sz, i + ii - sz));
                                }
                            }
                        }
                        if (is_dilation)
                        {
                            val = *std::max_element(list.begin(), list.end());
                        }
                        else
                        {
                            val = *std::min_element(list.begin(), list.end());
                        }
                    }
                    output.set_value(j, i, val);
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
bool Matrix<T>::isonboundary(size_t x, size_t y, size_t sz)
{
    return (!(x < (mRows - sz) && x >= sz) || !(y < (mCols - sz) && y >= sz));
}
