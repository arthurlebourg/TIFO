#pragma once

#include "matrix.hh"

enum Edge : uint8_t
{
    NONE,
    WEAK,
    STRONG = 255,
};

void intensity_gradients(Matrix<float> &input, Matrix<float> &gradient,
                         Matrix<float> &angle);

void non_maximum_suppression(Matrix<float> &gradient_in,
                             Matrix<float> &angle_in, Matrix<float> &output);

void weak_strong_edges_thresholding(Matrix<float> &input,
                                    Matrix<float> &output);

void weak_edges_removal(Matrix<float> &input, Matrix<float> &output);
