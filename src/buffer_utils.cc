#include "buffer_utils.hh"

#include <tbb/parallel_for.h>

size_t get_offset(size_t x, size_t y)
{
    return (y * 4) * screen_width + (x * 4);
}

Color get_pixel(unsigned char *raw_buffer, size_t offset)
{
    return Color(raw_buffer[offset + 0], raw_buffer[offset + 1],
                 raw_buffer[offset + 2], raw_buffer[offset + 3]);
}

void set_pixel(unsigned char *raw_buffer, size_t offset, Color &col)
{
    raw_buffer[offset + 0] = col.red();
    raw_buffer[offset + 1] = col.green();
    raw_buffer[offset + 2] = col.blue();
    raw_buffer[offset + 3] = col.a();
}

void to_rgb_matrix(unsigned char *raw_buffer, Matrix<Color> &output)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
                output.get_data()[i] = get_pixel(raw_buffer, i * 4);
        });
}

void boost_saturation(unsigned char *raw_buffer)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                Color value = get_pixel(raw_buffer, i * 4);
                float r = value.red() / 255.0;
                float g = value.green() / 255.0;
                float b = value.blue() / 255.0;

                float c_max = std::max(r, std::max(g, b));
                float c_min = std::min(r, std::min(g, b));

                float delta = c_max - c_min;

                float h = 0;
                if (delta == 0)
                {
                    h = 0;
                }
                else if (c_max == r)
                {
                    h = 60 * std::fmod((g - b) / delta, 6);
                }
                else if (c_max == g)
                {
                    h = 60 * ((b - r) / delta + 2);
                }
                else if (c_max == b)
                {
                    h = 60 * ((r - g) / delta + 4);
                }

                float s = c_max == 0 ? 0 : delta / c_max;

                float v = c_max;

                // saturation:
                s *= 1.5;

                // to rgb

                float c = v * s;
                float x = c * (1 - std::abs(std::fmod(h / 60.0, 2) - 1));
                float m = v - c;

                r = 0;
                g = 0;
                b = 0;
                if (h >= 0 && h < 60)
                {
                    r = c;
                    g = x;
                }
                else if (h >= 60 && h < 120)
                {
                    r = x;
                    g = c;
                }
                else if (h >= 120 && h < 180)
                {
                    g = c;
                    b = x;
                }
                else if (h >= 180 && h < 240)
                {
                    g = x;
                    b = c;
                }
                else if (h >= 240 && h < 300)
                {
                    r = x;
                    b = c;
                }
                else if (h >= 300 && h < 360)
                {
                    r = c;
                    b = x;
                }

                Color res((r + m) * 255, (g + m) * 255, (b + m) * 255, 255);
                set_pixel(raw_buffer, i * 4, res);
            }
        });
}

void fill_buffer(unsigned char *raw_buffer, Matrix<Color> &mat)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                Color value = mat.get_data()[i];
                set_pixel(raw_buffer, i * 4, value);
            }
        });
}

void apply_palette(unsigned char *raw_buffer, Quantizer &q,
                   std::vector<Color> &palette)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                Color color = get_pixel(raw_buffer, i * 4);
                size_t index = q.get_palette_index(color);
                Color new_color = palette[index];
                set_pixel(raw_buffer, i * 4, new_color);
            }
        });
}

void apply_palette_debug(unsigned char *raw_buffer, Quantizer &q,
                         std::vector<Color> &palette, size_t x_limit)
{
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, screen_height * screen_width),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                Color color = get_pixel(raw_buffer, i * 4);
                size_t index = q.get_palette_index(color);
                Color new_color =
                    (i % screen_width > x_limit) ? palette[index] : color;
                set_pixel(raw_buffer, i * 4, new_color);
            }
        });
}

void pixelate_buffer(unsigned char *raw_buffer, size_t pixel_size)
{
    for (size_t i = 0; i < screen_width; i += pixel_size)
    {
        for (size_t j = 0; j < screen_width; j += pixel_size)
        {
            size_t offset = get_offset(j, i);
            Color old_pixel = get_pixel(raw_buffer, offset);
            size_t red = 0;
            size_t blue = 0;
            size_t green = 0;
            for (size_t ii = 0; ii < pixel_size; ii++)
            {
                for (size_t jj = 0; jj < pixel_size; jj++)
                {
                    size_t offset_bis = get_offset(j + jj, i + ii);
                    Color next_pixel = get_pixel(raw_buffer, offset_bis);
                    red += next_pixel.red();
                    blue += next_pixel.blue();
                    green += next_pixel.green();
                }
            }
            red /= (pixel_size * pixel_size);
            blue /= (pixel_size * pixel_size);
            green /= (pixel_size * pixel_size);

            auto color = Color(red, green, blue, old_pixel.a());
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
