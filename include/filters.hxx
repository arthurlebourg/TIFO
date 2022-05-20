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
                    unsigned char value = (unsigned char)mat->at(j, i);
                    if (value > 0)
                        set_pixel(pixels, offset, Color(0, 0, 0, 255));
                }
            }
        });
}

template <typename T>
void fill_buffer_palette(size_t miny, size_t maxy, unsigned char *pixels,
                         Matrix<T> &mat, Quantizer q);
