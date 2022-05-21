#include <tbb/parallel_for.h>

#include "matrix.hh"

template <typename T>
void to_grayscale(unsigned char *raw_buffer, Matrix<T> &output)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                Color color = get_pixel(raw_buffer, i * 4);
                output.get_data()[i] = color.red() * 0.299
                    + color.green() * 0.587 + color.blue() * 0.114;
            }
        });
}

template <typename T>
void remap_to_rgb(Matrix<T> &mat)
{
    auto minmax = mat.get_minmax();
    T diff = minmax.second - minmax.first;

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                mat.get_data()[i] =
                    (int)((mat.get_data()[i] - minmax.first) * 255. / diff);
            }
        });
}

template <typename T>
void fill_buffer(unsigned char *raw_buffer, Matrix<T> &mat)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                unsigned char value = (unsigned char)mat.get_data()[i];
                Color c(value, value, value, 255);
                set_pixel(raw_buffer, i * 4, c);
            }
        });
}

template <typename T>
void set_dark_borders(unsigned char *raw_buffer, Matrix<T> &border_mask)
{
    auto border_color = Color(0, 0, 0, 255);

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                unsigned char value = (unsigned char)border_mask.get_data()[i];
                if (value > 0)
                    set_pixel(raw_buffer, i * 4, border_color);
            }
        });
}
