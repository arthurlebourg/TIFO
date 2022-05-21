#pragma once

#include "matrix.hh"

enum Edge : uint8_t
{
    NONE,
    WEAK,
    STRONG = 255,
};

void edge_detection(std::vector<Matrix<float>> &tmp_buffers);
