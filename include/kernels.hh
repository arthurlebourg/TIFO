#pragma once

#include "matrix.hh"

Matrix<float> ellipse_kernel(int height, int width);

Matrix<float> sobel_x();
Matrix<float> sobel_y();
