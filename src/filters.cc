#include "filters.hh"

std::vector<float> to_grayscale(unsigned char *pixels)
{
    std::vector<float> res(screen_width * screen_height);
    for (size_t y = 0; y < screen_height; y++)
    {
        for (size_t x = 0; x < screen_width; x++)
        {
            size_t offset = get_offset(x, y);
            Color old_pixel = get_pixel(pixels, offset);
            float gray = old_pixel.red() * 0.299 + old_pixel.green() * 0.587
                + old_pixel.blue() * 0.114;
            res[y * screen_width + x] = gray;
        }
    }
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

void fill_buffer_pixelate(size_t miny, size_t maxy, unsigned char *pixels,
                          size_t pixel_size)
{
    for (size_t y = miny; y < maxy; y += pixel_size)
    {
        for (size_t x = 0; x < screen_width; x += pixel_size)
        {
            size_t offset = get_offset(x, y);
            Color old_pixel = get_pixel(pixels, offset);
            size_t red = 0;
            size_t blue = 0;
            size_t green = 0;
            for (size_t i = 0; i < pixel_size; i++)
            {
                for (size_t j = 0; j < pixel_size; j++)
                {
                    size_t offset_bis = get_offset(x + i, y + j);
                    Color next_pixel = get_pixel(pixels, offset_bis);
                    red += next_pixel.red();
                    blue += next_pixel.blue();
                    green += next_pixel.green();
                }
            }
            red /= (pixel_size * pixel_size);
            blue /= (pixel_size * pixel_size);
            green /= (pixel_size * pixel_size);

            set_pixel(pixels, offset, Color(red, green, blue, old_pixel.a()));
            for (size_t i = 0; i < pixel_size; i++)
            {
                for (size_t j = 0; j < pixel_size; j++)
                {
                    size_t offset_bis = get_offset(x + i, y + j);
                    set_pixel(pixels, offset_bis,
                              Color(red, green, blue, old_pixel.a()));
                }
            }
        }
    }
}

void fill_buffer(size_t miny, size_t maxy, unsigned char *pixels)
{
    for (size_t y = miny; y < maxy; y++)
    {
        for (size_t x = 0; x < screen_width; x++)
        {
            size_t offset = get_offset(x, y);
            Color old_pixel = get_pixel(pixels, offset);

            set_pixel(pixels, offset, old_pixel);
        }
    }
}
