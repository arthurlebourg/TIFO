#include "filters.hh"

#include <tbb/parallel_for.h>

std::vector<float> to_grayscale(unsigned char *pixels)
{
    std::vector<float> res(screen_width * screen_height);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, screen_height),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = 0; j < screen_width; j++)
                              {
                                  size_t offset = get_offset(j, i);
                                  Color old_pixel = get_pixel(pixels, offset);
                                  float gray = old_pixel.red() * 0.299
                                      + old_pixel.green() * 0.587
                                      + old_pixel.blue() * 0.114;
                                  res[i * screen_width + j] = gray;
                              }
                          }
                      });
    return res;
}

Color get_pixel(unsigned char *pixels, size_t offset)
{
    return Color(pixels[offset + 0], pixels[offset + 1], pixels[offset + 2],
                 pixels[offset + 3]);
}

void set_pixel(unsigned char *pixels, size_t offset, Color col)
{
    pixels[offset + 0] = col.blue(); // b
    pixels[offset + 1] = col.green(); // g
    pixels[offset + 2] = col.red(); // r
    pixels[offset + 3] = col.a(); // a
}

size_t get_offset(size_t x, size_t y)
{
    return (y * 4) * screen_width + (x * 4);
}

void fill_buffer_pixelate(unsigned char *pixels, size_t pixel_size)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                for (size_t j = 0; j < screen_width; j += pixel_size)
                {
                    size_t offset = get_offset(j, i);
                    Color old_pixel = get_pixel(pixels, offset);
                    size_t red = 0;
                    size_t blue = 0;
                    size_t green = 0;
                    for (size_t i = 0; i < pixel_size; i++)
                    {
                        for (size_t j = 0; j < pixel_size; j++)
                        {
                            size_t offset_bis = get_offset(j + i, i + j);
                            Color next_pixel = get_pixel(pixels, offset_bis);
                            red += next_pixel.red();
                            blue += next_pixel.blue();
                            green += next_pixel.green();
                        }
                    }
                    red /= (pixel_size * pixel_size);
                    blue /= (pixel_size * pixel_size);
                    green /= (pixel_size * pixel_size);

                    set_pixel(pixels, offset,
                              Color(red, green, blue, old_pixel.a()));
                    for (size_t ii = 0; ii < pixel_size; ii++)
                    {
                        for (size_t jj = 0; jj < pixel_size; jj++)
                        {
                            size_t offset_bis = get_offset(j + ii, i + jj);
                            set_pixel(pixels, offset_bis,
                                      Color(red, green, blue, old_pixel.a()));
                        }
                    }
                }
            }
        });
}

void fill_buffer(unsigned char *pixels)
{
    tbb::parallel_for(tbb::blocked_range<size_t>(0, screen_height),
                      [&](tbb::blocked_range<size_t> r) {
                          for (size_t i = r.begin(); i < r.end(); i++)
                          {
                              for (size_t j = 0; j < screen_width; j++)
                              {
                                  size_t offset = get_offset(j, i);
                                  Color old_pixel = get_pixel(pixels, offset);

                                  set_pixel(pixels, offset, old_pixel);
                              }
                          }
                      });
}
