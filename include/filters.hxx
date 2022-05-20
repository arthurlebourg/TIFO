#include <tbb/parallel_for.h>

#include "matrix.hh"

template <typename T>
void fill_buffer(unsigned char *pixels, Matrix<T> *mat)
{
    tbb::parallel_for(tbb::blocked_range<size_t>(0, screen_height),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = 0; j < screen_width; j++)
                              {
                                  size_t offset = get_offset(j, i);
                                  unsigned char value =
                                      (unsigned char)mat->at(j, i);
                                  Color c(value, value, value, 255);
                                  set_pixel(pixels, offset, c);
                              }
                          }
                      });
}

template <typename T>
void fill_buffer_dark_borders(unsigned char *pixels, Matrix<T> *mat)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < screen_width; j++)
                {
                    size_t offset = get_offset(j, i);
                    Color old_pixel = get_pixel(pixels, offset);

                    unsigned char value = (unsigned char)mat->at(j, i);
                    unsigned char red = value ? 0 : old_pixel.red();
                    unsigned char green = value ? 0 : old_pixel.green();
                    unsigned char blue = value ? 0 : old_pixel.blue();
                    Color c(red, green, blue, 255);

                    set_pixel(pixels, offset, c);
                }
            }
        });
}
