#pragma once

#include "color.hh"
#include "matrix.hh"

void gaussian_blur(Matrix<float> &mat, Matrix<float> &tmp_buffer);

void bilateral_filter(Matrix<Color> &res, Matrix<Color> &source,
                      size_t diameter);
