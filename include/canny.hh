#include "matrix.hh"

void intensity_gradients(Matrix<float> &input, Matrix<float> &gradient_out,
                         Matrix<float> &angle_out);

void non_maximum_suppression(Matrix<float> &gradient_in,
                             Matrix<float> &angle_in, Matrix<float> &output);
