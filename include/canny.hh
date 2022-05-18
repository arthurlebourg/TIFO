#pragma once

#include "matrix.hh"

enum Edge : uint8_t
{
    NONE,
    WEAK,
    STRONG,
};

void intensity_gradients(Matrix<float> &input, Matrix<float> &gradient_out,
                         Matrix<float> &angle_out);

void non_maximum_suppression(Matrix<float> &gradient_in,
                             Matrix<float> &angle_in, Matrix<float> &output);

void weak_strong_edges_thresholding(Matrix<float> &input, Matrix<Edge> &output);

void weak_edges_removal(Matrix<Edge> &input, Matrix<float> &output);
