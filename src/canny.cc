#include "canny.hh"

#include <math.h>

const float SOBEL_X[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };

const float SOBEL_Y[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

void intensity_gradients(Matrix<float> &gray, Matrix<float> &gradient,
                         Matrix<float> &direction)
{
    auto m_rows = gray.get_rows();
    auto m_cols = gray.get_cols();

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
                        g_x += gray.at(jj, ii) * SOBEL_X[m * 3 + n];
                        g_y += gray.at(jj, ii) * SOBEL_Y[m * 3 + n];
                    }
                }
            }
            // Approximation: sqrt(Gx² + Gy²) -> |Gx| + |Gy|
            gradient.set_value(j, i, std::abs(g_x) + std::abs(g_y));
            direction.set_value(j, i, std::atan2(g_y, g_x));
        }
    }
}
