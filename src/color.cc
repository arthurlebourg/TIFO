#include "color.hh"

#include <algorithm>
#include <cmath>

HSV to_hsv(RGB &color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

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

    return HSV{ h, s, v };
}

RGB to_rgb(HSV &color)
{
    float c = color.v * color.s;
    float x = c * (1 - std::abs(std::fmod(color.h / 60.0, 2) - 1));
    float m = color.v - c;

    float r = 0, g = 0, b = 0;
    if (color.h >= 0 && color.h < 60)
    {
        r = c;
        g = x;
    }
    else if (color.h >= 60 && color.h < 120)
    {
        r = x;
        g = c;
    }
    else if (color.h >= 120 && color.h < 180)
    {
        g = c;
        b = x;
    }
    else if (color.h >= 180 && color.h < 240)
    {
        g = x;
        b = c;
    }
    else if (color.h >= 240 && color.h < 300)
    {
        r = x;
        b = c;
    }
    else if (color.h >= 300 && color.h < 360)
    {
        r = c;
        b = x;
    }

    return RGB{ (uint8_t) unsigned((r + m) * 255),
                (uint8_t) unsigned((g + m) * 255),
                (uint8_t) unsigned((b + m) * 255) };
}
