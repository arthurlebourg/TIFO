#include "kernels.hh"

Matrix<float> mgridy(float begin, float end)
{
    Matrix<float> mat(end - begin, end - begin);
    std::vector<float> values;
    for (float i = begin; i < end; i++)
    {
        for (float j = begin; j < end; j++)
        {
            values.push_back(i);
        }
    }
    mat.set_values(values);
    return mat;
}

Matrix<float> mgridx(float begin, float end)
{
    Matrix<float> mat(end - begin, end - begin);
    std::vector<float> values;
    for (float i = begin; i < end; i++)
    {
        for (float j = begin; j < end; j++)
        {
            values.push_back(j);
        }
    }
    mat.set_values(values);
    return mat;
}

Matrix<float> gauss_kernel(float size)
{
    Matrix<float> x = mgridx(-size, size + 1);
    Matrix<float> y = mgridy(-size, size + 1);

    float coef = 2 * (size / 3) * (size / 3);

    auto func = [coef](float a, size_t i) {
        i = i;
        return ((a * a) / coef);
    };
    x.apply(func);
    y.apply(func);
    x += y;
    auto func2 = [](float a, size_t i) {
        i = i;
        return exp(-a);
    };
    x.apply(func2);
    return x;
}

Matrix<float> derivative_gauss_kernel_x(float size)
{
    Matrix<float> x = mgridx(-size, size + 1);
    Matrix<float> kernel = gauss_kernel(size);

    x *= kernel;
    return -x;
}

Matrix<float> derivative_gauss_kernel_y(float size)
{
    Matrix<float> y = mgridy(-size, size + 1);
    Matrix<float> kernel = gauss_kernel(size);

    y *= kernel;
    return -y;
}

Matrix<float> ellipse_kernel(int height, int width)
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

            int dx =
                saturate_cast<int, float>(tmp, std::numeric_limits<int>::min(),
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

Matrix<float> square_kernel(int height, int width)
{
    return Matrix<float>(height, width, 1);
}

Matrix<float> sobel_x_kernel()
{
    return Matrix<float>(3, 3, { -1, 0, 1, -2, 0, 2, -1, 0, 1 });
}

Matrix<float> sobel_y_kernel()
{
    return Matrix<float>(3, 3, { -1, -2, -1, 0, 0, 0, 1, 2, 1 });
}

Matrix<float> sobel_x_vertical_kernel()
{
    return Matrix<float>(3, 1, { 1, 2, 1 });
}

Matrix<float> sobel_x_horizontal_kernel()
{
    return Matrix<float>(1, 3, { 1, 0, -1 });
}

Matrix<float> sobel_y_vertical_kernel()
{
    return Matrix<float>(3, 1, { 1, 0, -1 });
}

Matrix<float> sobel_y_horizontal_kernel()
{
    return Matrix<float>(1, 3, { 1, 2, 1 });
}
