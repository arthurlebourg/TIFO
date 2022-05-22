#include "canny.hh"

#include <iostream>
#include <math.h>
#include <tbb/parallel_for.h>

#include "filters.hh"

const float SOBEL_X[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
const float SOBEL_Y[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

void intensity_gradients(Matrix<float> &input, Matrix<float> &gradient_out,
                         Matrix<float> &angle_out)
{
    auto m_rows = input.get_rows();
    auto m_cols = input.get_cols();

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, m_rows),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < m_cols; j++)
                {
                    float g_x = 0;
                    float g_y = 0;
                    for (size_t m = 0; m < 3; m++)
                    {
                        for (size_t n = 0; n < 3; n++)
                        {
                            size_t ii = i + (-1 + m);
                            size_t jj = j + (-1 + n);

                            if (ii < m_rows && jj < m_cols)
                            {
                                g_x +=
                                    input.safe_at(jj, ii) * SOBEL_X[m * 3 + n];
                                g_y +=
                                    input.safe_at(jj, ii) * SOBEL_Y[m * 3 + n];
                            }
                        }
                    }

                    // Approximation: sqrt(Gx² + Gy²) => |Gx| + |Gy|
                    gradient_out.set_value(j, i, std::abs(g_x) + std::abs(g_y));
                    angle_out.set_value(j, i, std::atan2(g_y, g_x));
                }
            }
        });
}

void non_maximum_suppression(Matrix<float> &gradient_in,
                             Matrix<float> &angle_in, Matrix<float> &output)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, gradient_in.get_rows()),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < gradient_in.get_cols(); j++)
                {
                    float angle = angle_in.get_value(j, i) * 180 / M_PI;

                    if (angle < 0)
                        angle += 180;

                    float q = 256, r = 256;

                    // 0°
                    if (angle < 22.5 || angle >= 157.5)
                    {
                        r = gradient_in.safe_at(j - 1, i);
                        q = gradient_in.safe_at(j + 1, i);
                    }
                    // 45°
                    else if (angle >= 22.5 && angle < 67.5)
                    {
                        q = gradient_in.safe_at(j - 1, i - 1);
                        r = gradient_in.safe_at(j + 1, i + 1);
                    }
                    // 90°
                    else if (angle >= 67.5 && angle < 112.5)
                    {
                        r = gradient_in.safe_at(j, i - 1);
                        q = gradient_in.safe_at(j, i + 1);
                    }
                    // 135°
                    else if (angle >= 112.5 && angle < 157.5)
                    {
                        q = gradient_in.safe_at(j - 1, i + 1);
                        r = gradient_in.safe_at(j + 1, i - 1);
                    }

                    float value = gradient_in.get_value(j, i);
                    if (value >= q && value >= r)
                        output.set_value(j, i, value);
                    else
                        output.set_value(j, i, 0);
                }
            }
        });
}

void weak_strong_edges_thresholding(Matrix<float> &input, Matrix<float> &output,
                                    float lo, float hi)
{
    float high_threshold = input.get_max() * hi;
    float low_threshold = high_threshold * lo;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, input.get_rows()),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = 0; j < input.get_cols(); j++)
                              {
                                  float value = input.get_value(j, i);

                                  if (value >= high_threshold)
                                      output.set_value(j, i, STRONG);
                                  else if (value < low_threshold)
                                      output.set_value(j, i, NONE);
                                  else
                                      output.set_value(j, i, WEAK);
                              }
                          }
                      });
}

const std::pair<int8_t, int8_t> NEIGHBOURS[] = {
    { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 0 },
    { 1, 0 },   { -1, 1 }, { 0, 1 },  { 1, 1 },
};

void weak_edges_removal(Matrix<float> &input, Matrix<float> &output)
{
    tbb::parallel_for(tbb::blocked_range<size_t>(0, input.get_rows()),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = 0; j < input.get_cols(); j++)
                              {
                                  auto value = input.get_value(j, i);
                                  if (value == WEAK)
                                  {
                                      value = NONE;
                                      // If weak edge connected to strong edge
                                      for (auto p : NEIGHBOURS)
                                      {
                                          size_t ii = i + p.first;
                                          size_t jj = j + p.second;

                                          if (input.is_in_bound(jj, ii)
                                              && input.get_value(jj, ii)
                                                  == STRONG)
                                          {
                                              value = STRONG;
                                              break;
                                          }
                                      }
                                  }
                                  output.set_value(j, i, value);
                              }
                          }
                      });
}

void edge_detection(std::vector<Matrix<float>> &buffers, Blur blur)
{
    const float low_threshold_ratio = 0.030;
    const float hight_threshold_ratio = 0.150;

    edge_detection(buffers, blur, low_threshold_ratio, hight_threshold_ratio);
}

void edge_detection(std::vector<Matrix<float>> &buffers, Blur blur,
                    float low_threshold_ratio, float hight_threshold_ratio)
{
    switch (blur)
    {
    case Blur::NONE:
        break;
    case Blur::GAUSS:
        gaussian_blur(buffers[0], buffers[1]);
        gaussian_blur(buffers[1], buffers[0]);

        buffers[1].swap(buffers[0]);
        break;
    case Blur::MEDIAN:
        median_filter(buffers[0], buffers[1], 5);
        buffers[1].swap(buffers[0]);
        break;
    default:
        break;
    }

    intensity_gradients(buffers[0], buffers[1], buffers[2]);

    non_maximum_suppression(buffers[1], buffers[2], buffers[0]);

    weak_strong_edges_thresholding(buffers[0], buffers[1], low_threshold_ratio,
                                   hight_threshold_ratio);

    weak_edges_removal(buffers[1], buffers[0]);
}

void thicken_edges(Matrix<float> &edges_in, Matrix<float> &angle_in,
                   Matrix<float> &edges_out)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, edges_in.get_rows()),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < edges_in.get_cols(); j++)
                {
                    float value = edges_in.get_value(j, i);

                    if (value > 0.5)
                    {
                        float angle = angle_in.get_value(j, i) * 180 / M_PI;

                        if (angle < 0)
                            angle += 180;

                        // 0°
                        if (angle < 22.5 || angle >= 157.5)
                        {
                            edges_out.safe_set(j - 1, i, STRONG);
                            edges_out.safe_set(j + 1, i, STRONG);
                        }
                        // 45°
                        else if (angle >= 22.5 && angle < 67.5)
                        {
                            edges_out.safe_set(j - 1, i - 1, STRONG);
                            edges_out.safe_set(j + 1, i + 1, STRONG);
                        }
                        // 90°
                        else if (angle >= 67.5 && angle < 112.5)
                        {
                            edges_out.safe_set(j, i - 1, STRONG);
                            edges_out.safe_set(j, i + 1, STRONG);
                        }
                        // 135°
                        else if (angle >= 112.5 && angle < 157.5)
                        {
                            edges_out.safe_set(j - 1, i + 1, STRONG);
                            edges_out.safe_set(j + 1, i - 1, STRONG);
                        }
                    }

                    edges_out.set_value(j, i, value);
                }
            }
        });
}
