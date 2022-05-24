#include "filters.hh"

#include <iostream>
#include <math.h>
#include <tbb/parallel_for.h>

auto GAUSS_X = Matrix<float>(
    1, 5, { 0.02808743, 0.23430939, 0.47520637, 0.23430939, 0.02808743 });
auto GAUSS_Y = Matrix<float>(
    5, 1, { 0.02808743, 0.23430939, 0.47520637, 0.23430939, 0.02808743 });

float cGaussian[64];

void gaussian_blur(Matrix<float> &mat, Matrix<float> &tmp_buffer,
                   size_t padding)
{
    mat.convolve(GAUSS_X, tmp_buffer, padding);
    tmp_buffer.pad_borders(padding);
    tmp_buffer.convolve(GAUSS_Y, mat, padding);
    mat.pad_borders(padding);
}

float euclideanLen(RGB a, RGB b, float d)
{
    float mod = (b.r - a.r) * (b.r - a.r) + (b.g - a.g) * (b.g - a.g)
        + (b.b - a.b) * (b.b - a.b);

    return exp(-mod / (2.f * d * d));
}

float distance(int x, int y, int i, int j)
{
    return float(sqrt(pow(x - i, 2) + pow(y - j, 2)));
}

double gaussian(float x, double sigma)
{
    return exp(-(pow(x, 2)) / (2 * pow(sigma, 2))) / (2 * M_PI * pow(sigma, 2));
}

void updateGaussian(float delta, int radius)
{
    for (int i = 0; i < 2 * radius + 1; ++i)
    {
        float x = i - radius;
        cGaussian[i] = expf(-(x * x) / (2 * delta * delta));
    }
}

void apply_bilateral_filter(Matrix<float> &input, Matrix<float> &output,
                            size_t x, size_t y, size_t diameter, double sigmaI,
                            double sigmaS)
{
    double iFiltered = 0;
    double wP = 0;
    int neighbor_x = 0;
    int neighbor_y = 0;
    int radius = diameter / 2;

    for (size_t i = 0; i < diameter; i++)
    {
        for (size_t j = 0; j < diameter; j++)
        {
            neighbor_x = x - (radius - i);
            neighbor_y = y - (radius - j);
            double gi = gaussian(input.safe_at(neighbor_x, neighbor_y)
                                     - input.safe_at(x, y),
                                 sigmaI);
            double gs =
                gaussian(distance(x, y, neighbor_x, neighbor_y), sigmaS);
            double w = gi * gs;
            iFiltered = iFiltered + input.safe_at(neighbor_x, neighbor_y) * w;
            wP = wP + w;
        }
    }
    iFiltered = iFiltered / wP;
    output.set_value(x, y, input.get_value(x, y));
}

void apply_bilateral_filter(Matrix<RGB> &input, Matrix<RGB> &output, size_t x,
                            size_t y, int radius)
{
    float sum = 0.0f;
    float factor;
    RGB t{};
    RGB center = input.safe_at(x, y);

    for (int i = -radius; i <= radius; i++)
    {
        for (int j = -radius; j <= radius; j++)
        {
            RGB curPix = input.safe_at(x + j, y + i);
            factor = cGaussian[i + radius] * cGaussian[j + radius]
                * // domain factor
                euclideanLen(curPix, center, 0.1f); // range factor

            t = t + curPix * factor;
            sum += factor;
        }
    }

    // od[y * w + x] = rgbaFloatToInt(t / sum);
    output.set_value(x, y, t / sum);
}

void bilateral_filter(Matrix<float> &input, Matrix<float> &output, int diameter,
                      double sigmaI, double sigmaS)
{
    size_t width = input.get_cols();
    size_t height = input.get_rows();

    auto radius = diameter / 2;

    tbb::parallel_for(tbb::blocked_range<size_t>(radius, height - radius),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = radius; j < width - radius; j++)
                              {
                                  apply_bilateral_filter(input, output, j, i,
                                                         diameter, sigmaI,
                                                         sigmaS);
                              }
                          }
                      });
}

void bilateral_filter(Matrix<RGB> &input, Matrix<RGB> &output, size_t radius,
                      double delta)
{
    size_t width = input.get_cols();
    size_t height = input.get_rows();

    updateGaussian(delta, radius);

    tbb::parallel_for(tbb::blocked_range<size_t>(radius, height - radius),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = radius; j < width - radius; j++)
                              {
                                  apply_bilateral_filter(input, output, j, i,
                                                         radius);
                              }
                          }
                      });
}
