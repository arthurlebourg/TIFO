#include "color.hh"
#include "image.hh"

const size_t screen_width = 1280;
const size_t screen_height = 720;

std::vector<float> to_grayscale(unsigned char *pixels);

Color get_pixel(unsigned char *pixels, size_t offset);

void set_pixel(unsigned char *pixels, size_t offset, Color col);

size_t get_offset(size_t x, size_t y);

void fill_buffer_pixelate(size_t miny, size_t maxy, unsigned char *pixels,
                          size_t pixel_size);

void fill_buffer(size_t miny, size_t maxy, unsigned char *pixels);

void fill_buffer_gradient(size_t miny, size_t maxy, Matrix<float> mat,
                          unsigned char *pixels);
