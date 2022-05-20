#pragma once

#include "matrix.hh"

void gaussian_blur(Matrix<float> &input, Matrix<float> &tmp_buffer,
                   Matrix<float> &output);
