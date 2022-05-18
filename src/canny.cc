#include "canny.hh"

#include <math.h>

const float SOBEL_X[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };

const float SOBEL_Y[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

void intensity_gradients(Matrix<float> &input, Matrix<float> &gradient,
                         Matrix<float> &angle)
{
    auto m_rows = input.get_rows();
    auto m_cols = input.get_cols();

    for (size_t i = 0; i < m_rows; i++)
    {
        for (size_t j = 0; j < m_cols; j++)
        {
            float g_x = 0;
            float g_y = 0;
            for (size_t m = 0; m < 3; m++)
            {
                for (size_t n = 0; n < 3; n++)
                {
                    size_t ii = i - m - 1;
                    size_t jj = j - n - 1;

                    if (ii < m_rows && jj < m_cols)
                    {
                        g_x += input.at(jj, ii) * SOBEL_X[m * 3 + n];
                        g_y += input.at(jj, ii) * SOBEL_Y[m * 3 + n];
                    }
                }
            }

            // Approximation: sqrt(Gx² + Gy²) -> |Gx| + |Gy|
            gradient.set_value(j, i, std::abs(g_x) + std::abs(g_y));
            angle.set_value(j, i, std::atan2(g_y, g_x));
        }
    }
}

void non_maximum_suppression(Matrix<float> &gradient_in,
                             Matrix<float> &angle_in, Matrix<float> &output)
{
    auto m_rows = gradient_in.get_rows();
    auto m_cols = gradient_in.get_cols();

    for (size_t i = 0; i < m_rows; i++)
    {
        for (size_t j = 0; j < m_cols; j++)
        {
            float angle = angle_in.at(j, i);

            if (angle < 0)
                angle += 180;

            float q = 255, r = 255;

            // 0°
            if ((angle >= 0 && angle < 22.5)
                || (angle >= 157.5 && angle <= 180))
            {
                q = gradient_in.at(j, i + 1);
                r = gradient_in.at(j, i - 1);
            }
            // 45°
            else if (angle >= 22.5 && angle < 67.5)
            {
                q = gradient_in.at(j + 1, i - 1);
                r = gradient_in.at(j - 1, i + 1);
            }
            // 90°
            else if (angle >= 67.5 && angle < 112.5)
            {
                q = gradient_in.at(j + 1, i);
                r = gradient_in.at(j - 1, i);
            }
            // 135°
            else if (angle >= 112.5 && angle < 157.5)
            {
                q = gradient_in.at(j - 1, i - 1);
                r = gradient_in.at(j + 1, i + 1);
            }

            float value = gradient_in.at(j, i);
            if (value >= q && value >= r)
                output.set_value(j, i, value);
            else
                output.set_value(j, i, 0);
        }
    }
}
