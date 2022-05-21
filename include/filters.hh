#pragma once
#include <set>

#include "color.hh"
#include "matrix.hh"
#include "octree.hh"

const size_t screen_width = 1280;
const size_t screen_height = 720;

std::vector<float> to_grayscale(unsigned char *pixels);

Color get_pixel(unsigned char *pixels, size_t offset);

void set_pixel(unsigned char *pixels, size_t offset, Color col);

size_t get_offset(size_t x, size_t y);

void fill_buffer_pixelate(unsigned char *pixels, size_t pixel_size);

void fill_buffer(unsigned char *pixels);

void fill_buffer_palette(Quantizer q, std::vector<Color> palette,
                         unsigned char *pixels);

void fill_buffer_palette_debug(size_t miny, size_t maxy, Quantizer q,
                               std::vector<Color> palette,
                               unsigned char *pixels);
template <typename T>
void fill_buffer(unsigned char *pixels, Matrix<T> &mat);

template <typename T>
void fill_buffer_dark_borders(unsigned char *pixels, Matrix<T> &mat);

std::vector<Color> unique_colors(unsigned char *pixels);

template <typename T>
void remap_to_rgb(Matrix<T> &mat);

#include "filters.hxx"
