#include "filters.hh"

#include <iostream>

auto GAUSS_X = Matrix<float>(
    1, 5, { 0.02808743, 0.23430939, 0.47520637, 0.23430939, 0.02808743 });
auto GAUSS_Y = Matrix<float>(
    5, 1, { 0.02808743, 0.23430939, 0.47520637, 0.23430939, 0.02808743 });

void gaussian_blur(Matrix<float> &mat, Matrix<float> &tmp_buffer)
{
    mat.convolve(GAUSS_X, tmp_buffer);
    tmp_buffer.convolve(GAUSS_Y, mat);
}

float euclideanLen(Color a, Color b, float d)
{
    float mod = (b.red() - a.red()) * (b.red() - a.red())
        + (b.green() - a.green()) * (b.green() - a.green())
        + (b.blue() - a.blue()) * (b.blue() - a.blue());

    return exp(-mod / (2.f * d * d));
}

void updateGaussian(float delta, int radius, float *fGaussian)
{
    for (int i = 0; i < 2 * radius + 1; ++i)
    {
        float x = i - radius;
        fGaussian[i] = expf(-(x * x) / (2 * delta * delta));
    }
}

void apply_bilateral_filter(Matrix<Color> &res, Matrix<Color> &source, size_t x,
                            size_t y, int r)
{
    float sum = 0.0f;
    float factor;
    Color t;
    Color center = source.safe_at(x, y);
    float cGaussian[64];
    updateGaussian(4, 5, cGaussian);

    for (int i = -r; i <= r; i++)
    {
        for (int j = -r; j <= r; j++)
        {
            Color curPix = source.safe_at(x + j, y + i);
            factor = cGaussian[i + r] * cGaussian[j + r] * // domain factor
                euclideanLen(curPix, center, 0.1f); // range factor

            t = t + curPix * factor;
            sum += factor;
        }
    }

    // od[y * w + x] = rgbaFloatToInt(t / sum);
    res.set_value(x, y, t / sum);
}

void bilateral_filter(Matrix<Color> &res, Matrix<Color> &source,
                      size_t diameter)
{
    size_t width = source.get_cols();
    size_t height = source.get_rows();

    for (size_t i = 2; i < height - 2; i++)
    {
        for (size_t j = 2; j < width - 2; j++)
        {
            apply_bilateral_filter(res, source, i, j, diameter);
        }
    }
}
