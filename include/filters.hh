#pragma once

#include "color.hh"
#include "matrix.hh"

extern float cGaussian[64];

void gaussian_blur(Matrix<float> &input_output, Matrix<float> &tmp_buffer,
                   size_t padding);

template <typename T>
void median_filter(Matrix<T> &mat, size_t window_size);

void updateGaussian(float delta, int radius);

void bilateral_filter(Matrix<float> &input, Matrix<float> &output, int diameter,
                      double sigmaI, double sigmaS);

void bilateral_filter(Matrix<RGB> &input, Matrix<RGB> &output, size_t radius,
                      double delta);

#include "filters.hxx"
