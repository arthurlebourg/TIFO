#include "matrix.hh"

template <typename T>
void Matrix<T>::set_values(std::vector<T> val)
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

template <typename T>
T Matrix<T>::get_min()
{
    T min = 0;
    for (size_t i = 0; i < mRows * mCols; i++)
    {
        if (mData[i] < min)
            min = mData[i];
    }
    return min;
}

template <typename T>
T Matrix<T>::get_max()
{
    T max = 0;
    for (size_t i = 0; i < mRows * mCols; i++)
    {
        if (mData[i] > max)
            max = mData[i];
    }
    return max;
}

template <typename T>
void Matrix<T>::apply(const std::function<T(T, size_t)> &func)
{
    for (size_t y = 0; y < mRows; y++)
    {
        for (size_t x = 0; x < mCols; x++)
        {
            mData[y * mCols + x] = func(mData[y * mCols + x], y * mCols + x);
        }
    }
}

template <typename T>
Matrix<T> Matrix<T>::convolve(Matrix<T> kernel)
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

template <typename T>
Matrix<T> Matrix<T>::morph(Matrix<T> kernel, bool is_dilation)
{
    Matrix<T> res = Matrix<T>(mRows, mCols);
    float val;
    size_t sz = (kernel.get_rows() - 1) / 2;
    for (size_t x = 0; x < mRows; x++)
    {
        for (size_t y = 0; y < mCols; y++)
        {
            if (isonboundary(x, y, sz))
            {
                val = 0;
            }
            else
            {
                std::vector<float> list;
                for (size_t i = 0; i < kernel.get_rows(); i++)
                {
                    for (size_t j = 0; j < kernel.get_cols(); j++)
                    {
                        if (kernel.at(j, i) == 1)
                        {
                            list.push_back(at(y + j - sz, x + i - sz));
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
            res.set_value(y, x, val);
        }
    }
    return res;
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
std::vector<T> Matrix<T>::get_mData()
{
    return mData;
}

template <typename T>
T Matrix<T>::at(size_t x, size_t y)
{
    return mData[y * mCols + x];
}

template <typename T>
void Matrix<T>::set_value(size_t x, size_t y, T val)
{
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
    std::vector<T> new_vec;
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        new_vec.push_back(mData[i] * rhs.mData[i]);
    }
    res.set_values(new_vec);
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
    std::vector<T> new_vec;
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        new_vec.push_back(mData[i] / rhs.mData[i]);
    }
    res.set_values(new_vec);
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
    std::vector<T> new_vec;
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        new_vec.push_back(mData[i] + rhs.mData[i]);
    }
    res.set_values(new_vec);
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
    std::vector<T> new_vec;
    for (size_t i = 0; i < rhs.mCols * rhs.mRows; i++)
    {
        new_vec.push_back(mData[i] - rhs.mData[i]);
    }
    res.set_values(new_vec);
    return res;
}

template <typename T>
bool Matrix<T>::isonboundary(size_t x, size_t y, size_t sz)
{
    return (!(x < (mRows - sz) && x >= sz) || !(y < (mCols - sz) && y >= sz));
}
