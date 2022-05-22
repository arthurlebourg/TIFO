#pragma once
#include <set>

#include "color.hh"
#include "matrix.hh"
#include "octree.hh"

const size_t screen_width = 1280;
const size_t screen_height = 720;

/*
 * Get raw buffer offset
 */
size_t get_offset(size_t x, size_t y);

/*
 * Read raw pixel as RGBA
 */
Color get_pixel(unsigned char *raw_buffer, size_t offset);

/*
 * Set raw pixel as RGBA
 */
void set_pixel(unsigned char *raw_buffer, size_t offset, Color &col);

/*
 * Make a Matrix out of buffer
 */
void to_rgb_matrix(unsigned char *raw_buffer, Matrix<Color> &output);

/*
 * Get grayscale matrix from RGBA input buffer
 */
template <typename T>
void to_grayscale(unsigned char *raw_buffer, Matrix<T> &output);

/*
 * Converts to HSV, then boosts luminance, to converts back to RGB
 */
void boost_luminance(unsigned char *raw_buffer);

/*
 * Remap matrix values to RGB range (0-255)
 */
template <typename T>
void remap_to_rgb(Matrix<T> &mat);

/*
 * Fill buffer using matrix values (assumed to be in RGB range)
 */
void fill_buffer(unsigned char *raw_buffer, Matrix<Color> &mat);
template <typename T>
void fill_buffer(unsigned char *raw_buffer, Matrix<T> &mat);

/*
 * Apply new color palette
 */
void apply_palette(unsigned char *raw_buffer, Quantizer &q,
                   std::vector<Color> &palette);

/*
 * Apply new color palette only in [0; x_limit] range
 */
void apply_palette_debug(unsigned char *raw_buffer, Quantizer &q,
                         std::vector<Color> &palette, size_t x_limit);

/*
 * Set detected borders in black
 */
template <typename T>
void set_dark_borders(unsigned char *raw_buffer, Matrix<T> &border_mask);

/*
 * Simple pixelation filter
 */
void pixelate_buffer(unsigned char *raw_buffer, size_t pixel_size);

#include "buffer_utils.hxx"
