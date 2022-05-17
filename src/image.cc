#include "image.hh"

GrayImage GrayImage::operator*(GrayImage &img)
{
    auto new_mat = (*channel) * (*img.channel);
    auto new_channel = std::make_shared<Matrix<float>>(new_mat);
    return GrayImage(new_channel);
}

GrayImage GrayImage::morph(bool is_dilation)
{
    GrayImage res = GrayImage(std::make_shared<Matrix<float>>(height_, width_));
    Matrix<float> kernel = ellipse_kernel(25, 25);
    float val;
    size_t sz = (kernel.get_rows() - 1) / 2;
    for (size_t x = 0; x < height_; x++)
    {
        for (size_t y = 0; y < width_; y++)
        {
            if (channel->isonboundary(x, y, sz))
            {
                val = 0;
            }
            else
            {
                std::vector<float> list;
                for (size_t i = 0; i < kernel.get_rows(); i++)
                {
                    for (size_t j = 0; j < kernel.get_cols(); j++)
                    {
                        if (kernel.at(j, i) == 1)
                        {
                            list.push_back(channel->at(y + j - sz, x + i - sz));
                        }
                    }
                }
                if (is_dilation)
                {
                    val = *std::max_element(list.begin(), list.end());
                }
                else
                {
                    val = *std::min_element(list.begin(), list.end());
                }
            }
            res.channel->set_value(y, x, val);
        }
    }
    return res;
}

void GrayImage::threshold(float thresh)
{
    float max = channel->get_max();
    float min = channel->get_min();
    for (size_t i = 0; i < width_ * height_; i++)
    {
        if (channel->at(i, 0) <= min + thresh * (max - min))
        {
            channel->set_value(i, 0, 0);
        }
    }
}

size_t GrayImage::get_width()
{
    return width_;
}

size_t GrayImage::get_height()
{
    return height_;
}
