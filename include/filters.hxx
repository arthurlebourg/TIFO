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

template <typename T>
void fill_buffer_dark_borders(size_t miny, size_t maxy, unsigned char *pixels,
                              Matrix<T> &mat)
{
    for (size_t y = miny; y < maxy; y++)
    {
        for (size_t x = 0; x < screen_width; x++)
        {
            size_t offset = get_offset(x, y);
            Color old_pixel = get_pixel(pixels, offset);

            unsigned char value = (unsigned char)mat.at(x, y);
            unsigned char red = value ? 0 : old_pixel.red();
            unsigned char green = value ? 0 : old_pixel.green();
            unsigned char blue = value ? 0 : old_pixel.blue();
            Color c(red, green, blue, 255);

            set_pixel(pixels, offset, c);
        }
    }
}

template <typename T>
void fill_buffer_palette(size_t miny, size_t maxy, unsigned char *pixels,
                         Matrix<T> &mat, Quantizer q);
