#include "buffer_utils.hh"

#include <tbb/parallel_for.h>

size_t get_offset(size_t x, size_t y)
{
    return (y * 4) * screen_width + (x * 4);
}

RGB get_pixel(unsigned char *raw_buffer, size_t offset)
{
    return RGB(raw_buffer[offset + 0], raw_buffer[offset + 1],
               raw_buffer[offset + 2]); // skip alpha channel
}

void set_pixel(unsigned char *raw_buffer, size_t offset, RGB &col)
{
    raw_buffer[offset + 0] = col.r;
    raw_buffer[offset + 1] = col.g;
    raw_buffer[offset + 2] = col.b;
    raw_buffer[offset + 3] = 255;
}

void to_rgb_matrix(unsigned char *raw_buffer, Matrix<RGB> &output)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
                output.get_data()[i] = get_pixel(raw_buffer, i * 4);
        });
}

void saturation_modification(unsigned char *raw_buffer,
                             const double saturation_factor)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                auto color = get_pixel(raw_buffer, i * 4);
                auto hsv = to_hsv(color);

                hsv.s *= saturation_factor;
                if (hsv.s > 1.)
                    hsv.s = 1.;

                auto new_color = to_rgb(hsv);
                set_pixel(raw_buffer, i * 4, new_color);
            }
        });
}

tbb::concurrent_vector<size_t>
compute_lightness_cumul_histogram(unsigned char *raw_buffer)
{
    tbb::concurrent_vector<size_t> histo(256, 0u);
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                auto color = get_pixel(raw_buffer, i * 4);
                auto hsv = to_hsv(color);
                histo[hsv.v * 255]++;
            }
        });

    for (size_t i = 1; i < 256; i++)
        histo[i] += histo[i - 1];
    return histo;
}

void contrast_correction(unsigned char *raw_buffer,
                         tbb::concurrent_vector<size_t> &cum_histo)
{
    auto cdf_min = cum_histo[0];
    for (size_t i = 0; i < cum_histo.size(); i++)
    {
        if (cum_histo[i] > 0)
        {
            cdf_min = cum_histo[0];
            break;
        }
    }

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                auto color = get_pixel(raw_buffer, i * 4);
                auto hsv = to_hsv(color);

                hsv.v = (float)(cum_histo[hsv.v * 255] - cdf_min)
                    / (screen_height * screen_width - cdf_min);

                auto new_color = to_rgb(hsv);
                set_pixel(raw_buffer, i * 4, new_color);
            }
        });
}

void fill_buffer(unsigned char *raw_buffer, Matrix<RGB> &mat)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                RGB value = mat.get_data()[i];
                set_pixel(raw_buffer, i * 4, value);
            }
        });
}

void apply_palette(unsigned char *raw_buffer, Quantizer &q,
                   std::vector<RGB> &palette)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                RGB color = get_pixel(raw_buffer, i * 4);
                size_t index = q.get_palette_index(color);
                RGB new_color = palette[index];
                set_pixel(raw_buffer, i * 4, new_color);
            }
        });
}

void apply_palette_debug(unsigned char *raw_buffer, Quantizer &q,
                         std::vector<RGB> &palette, size_t x_limit)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                RGB color = get_pixel(raw_buffer, i * 4);
                size_t index = q.get_palette_index(color);
                RGB new_color =
                    (i % screen_width > x_limit) ? palette[index] : color;
                set_pixel(raw_buffer, i * 4, new_color);
            }
        });
}

void pixelate_buffer(unsigned char *raw_buffer, size_t pixel_size)
{
    for (size_t i = 0; i < screen_height; i += pixel_size)
    {
        for (size_t j = 0; j < screen_width; j += pixel_size)
        {
            size_t offset = get_offset(j, i);
            size_t red = 0;
            size_t blue = 0;
            size_t green = 0;
            for (size_t ii = 0; ii < pixel_size; ii++)
            {
                for (size_t jj = 0; jj < pixel_size; jj++)
                {
                    size_t offset_bis = get_offset(j + jj, i + ii);
                    RGB next_pixel = get_pixel(raw_buffer, offset_bis);
                    red += next_pixel.r;
                    blue += next_pixel.b;
                    green += next_pixel.g;
                }
            }
            red /= (pixel_size * pixel_size);
            blue /= (pixel_size * pixel_size);
            green /= (pixel_size * pixel_size);

            auto color = RGB(red, green, blue);
            set_pixel(raw_buffer, offset, color);
            for (size_t ii = 0; ii < pixel_size; ii++)
            {
                for (size_t jj = 0; jj < pixel_size; jj++)
                {
                    size_t offset_bis = get_offset(j + jj, i + ii);
                    set_pixel(raw_buffer, offset_bis, color);
                }
            }
        }
    }
}
