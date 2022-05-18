#include "matrix.hh"

template <typename T>
void fill_buffer(size_t miny, size_t maxy, unsigned char *pixels,
                 Matrix<T> &mat)
{
    for (size_t y = miny; y < maxy; y++)
    {
        for (size_t x = 0; x < screen_width; x++)
        {
            size_t offset = get_offset(x, y);
            unsigned char value = (unsigned char)mat.at(x, y);
            Color c(value, value, value, 255);
            set_pixel(pixels, offset, c);
        }
    }
}
