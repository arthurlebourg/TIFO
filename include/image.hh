#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include "kernels.hh"
#include "matrix.hh"

class GrayImage
{
public:
    GrayImage(std::shared_ptr<Matrix<float>> c)
    {
        height_ = c->get_rows();
        width_ = c->get_cols();
        channel = c;
    }

    GrayImage operator*(GrayImage &img);

    GrayImage morph(bool is_dilation);

    GrayImage keep_local_max(GrayImage img);

    void threshold(float thresh);

    size_t get_width();

    size_t get_height();

    std::shared_ptr<Matrix<float>> channel;

private:
    size_t width_;
    size_t height_;
};
