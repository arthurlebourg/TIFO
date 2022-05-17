#include "kernels.hh"

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

Matrix<float> sobel_x()
{
    std::vector<float> values{ -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    return Matrix<float>(3, 3, values);
}

Matrix<float> sobel_y()
{
    std::vector<float> values{ -1, -2, -1, 0, 0, 0, 1, 2, 1 };
    return Matrix<float>(3, 3, values);
}
