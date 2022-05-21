#include <tbb/parallel_for.h>

#include "matrix.hh"

template <typename T>
void fill_buffer(unsigned char *pixels, Matrix<T> &mat)
{
    tbb::parallel_for(tbb::blocked_range<size_t>(0, screen_height),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = 0; j < screen_width; j++)
                              {
                                  size_t offset = get_offset(j, i);
                                  unsigned char value =
                                      (unsigned char)mat.get_value(j, i);
                                  Color c(value, value, value, 255);
                                  set_pixel(pixels, offset, c);
                              }
                          }
                      });
}

template <typename T>
void fill_buffer_dark_borders(unsigned char *pixels, Matrix<T> &mat)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < screen_width; j++)
                {
                    size_t offset = get_offset(j, i);
                    unsigned char value = (unsigned char)mat.get_value(j, i);
                    if (value > 0)
                        set_pixel(pixels, offset, Color(0, 0, 0, 255));
                }
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
