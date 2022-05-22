#pragma once

#include <iostream>

struct RGB
{
    size_t r, g, b;

    RGB()
        : r(0)
        , g(0)
        , b(0)
    {}

    RGB(size_t red, size_t green, size_t blue)
        : r(red)
        , g(green)
        , b(blue)
    {}

    RGB normalized(size_t pixel_count)
    {
        RGB res(r / pixel_count, g / pixel_count, b / pixel_count);
        return res;
    }

    RGB normalized_biased(size_t pixel_count)
    {
        RGB res(r / pixel_count, g / pixel_count, b / pixel_count);
        size_t max = 0;
        size_t min = 0;
        if (res.r > res.b)
        {
            if (res.r > res.g)
                max = res.r;
            else
                max = res.g;

            if (res.b > res.g)
                min = res.g;
            else
                min = res.b;
        }
        else
        {
            if (res.b > res.g)
                max = res.b;
            else
                max = res.g;

            if (res.r > res.g)
                min = res.g;
            else
                min = res.r;
        }

        if (res.r > res.b)
        {
            if (res.r > res.g)
                res.r = max - min + res.r > 255 ? 255 : max - min + res.r;
            else
                res.g = max - min + res.g > 255 ? 255 : max - min + res.g;

            if (res.b > res.g)
                res.g -= max - min > res.g ? res.g : max - min;
            else
                res.b -= max - min > res.b ? res.b : max - min;
        }
        else
        {
            if (res.b > res.g)
                res.b = max - min + res.b > 255 ? 255 : max - min + res.b;
            else
                res.g = max - min + res.g > 255 ? 255 : max - min + res.g;

            if (res.r > res.g)
                res.g -= max - min > res.g ? res.g : max - min;
            else
                res.r -= max - min > res.r ? res.r : max - min;
        }
        return res;
    }
};

struct HSV
{
    float h, s, v;
};

RGB to_rgb(HSV &color);
HSV to_hsv(RGB &color);

inline std::ostream &operator<<(std::ostream &os, RGB &col)
{
    return os << "(R: " << col.r << ", G: " << col.g << ", B: " << col.b
              << ") hex: " << std::hex << col.r << col.g << col.b << std::dec
              << std::endl;
}

inline RGB operator+(const RGB &c, size_t t)
{
    size_t red = t + c.r;
    size_t green = t + c.g;
    size_t blue = t + c.b;
    return RGB(red, green, blue);
}

inline RGB operator+(const RGB &lhs, const RGB &rhs)
{
    size_t red = lhs.r + rhs.r;
    size_t green = lhs.g + rhs.g;
    size_t blue = lhs.b + rhs.b;
    return RGB(red, green, blue);
}

inline RGB operator-(const RGB &lhs, const RGB &rhs)
{
    size_t red = lhs.r - rhs.r;
    size_t green = lhs.g - rhs.g;
    size_t blue = lhs.b - rhs.b;
    // prevents signed overflow
    return RGB(red > 255 ? 0 : red, green > 0 ? 255 : green,
               blue > 255 ? 0 : blue);
}

inline RGB operator*(const RGB &c, double t)
{
    if (t <= 0)
        return RGB(0, 0, 0);

    int red = t * c.r;
    int green = t * c.g;
    int blue = t * c.b;
    // prevents signed overflow
    return RGB(red > 255 ? 255 : red, green > 255 ? 255 : green,
               blue > 255 ? 255 : blue);
}

inline RGB operator/(const RGB &c, double t)
{
    if (t <= 0)
        return RGB(0, 0, 0);

    int red = c.r / t;
    int green = c.g / t;
    int blue = c.b / t;
    // prevents signed overflow
    return RGB(red > 255 ? 255 : red, green > 255 ? 255 : green,
               blue > 255 ? 255 : blue);
}
