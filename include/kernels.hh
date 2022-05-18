#pragma once

#include "matrix.hh"

Matrix<float> mgridy(float begin, float end);

Matrix<float> mgridx(float begin, float end);

Matrix<float> gauss_kernel(float size);

Matrix<float> derivative_gauss_kernel_x(float size);

Matrix<float> derivative_gauss_kernel_y(float size);

Matrix<float> derivative_gauss_x(Matrix<float> img, float size);

Matrix<float> derivative_gauss_y(Matrix<float> img, float size);

Matrix<float> ellipse_kernel(int height, int width);
Matrix<float> square_kernel(int height, int width);
