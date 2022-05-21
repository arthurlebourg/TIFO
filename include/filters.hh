#pragma once

#include "color.hh"
#include "matrix.hh"

void gaussian_blur(Matrix<float> &input_output, Matrix<float> &tmp_buffer);

template <typename T>
void median_filter(Matrix<T> &mat, size_t window_size);

void bilateral_filter(Matrix<Color> &res, Matrix<Color> &source,
                      size_t diameter);

#include "filters.hxx"
