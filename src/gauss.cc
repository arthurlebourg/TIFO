#include "gauss.hh"

#include <iostream>

auto GAUSS_X = Matrix<float>(
    1, 5, { 0.02808743, 0.23430939, 0.47520637, 0.23430939, 0.02808743 });
auto GAUSS_Y = Matrix<float>(
    5, 1, { 0.02808743, 0.23430939, 0.47520637, 0.23430939, 0.02808743 });

void gaussian_blur(Matrix<float> &input, Matrix<float> &tmp_buffer,
                   Matrix<float> &output)
{
    input.convolve(GAUSS_X, tmp_buffer);
    tmp_buffer.convolve(GAUSS_Y, output);
}
