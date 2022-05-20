#pragma once

#include <iostream>

class Color
{
public:
    // default black color
    Color()
        : red_(0)
        , green_(0)
        , blue_(0)
        , a_(0)
    {}

    Color(unsigned int red, unsigned int green, unsigned int blue,
          unsigned int a)
        : red_(red)
        , green_(green)
        , blue_(blue)
        , a_(a)
    {}

    unsigned int red() const
    {
        return red_;
    }

    unsigned int green() const
    {
        return green_;
    }

    unsigned int blue() const
    {
        return blue_;
    }

    unsigned int a() const
    {
        return a_;
    }

    Color normalized(size_t pixel_count)
    {
        return Color(red_ / pixel_count, green_ / pixel_count,
                     blue_ / pixel_count, a_);
    }

private:
    unsigned int red_;
    unsigned int green_;
    unsigned int blue_;
    unsigned int a_;
};

inline std::ostream &operator<<(std::ostream &os, Color &col)
{
    return os << "(R: " << col.red() << ", G: " << col.green()
              << ", B: " << col.blue() << ", A: " << col.a()
              << ") hex: " << std::hex << col.red() << col.green() << col.blue()
              << std::dec << std::endl;
}

inline Color operator+(const Color &c, unsigned int t)
{
    unsigned int red = t + c.red();
    unsigned int green = t + c.green();
    unsigned int blue = t + c.blue();
    // prevents signed overflow
    // return Color(red > 255 ? 255 : red, green > 255 ? 255 : green,
    //             blue > 255 ? 255 : blue, c.a());
    return Color(red, green, blue, c.a());
}

inline Color operator+(const Color &lhs, const Color &rhs)
{
    unsigned int red = lhs.red() + rhs.red();
    unsigned int green = lhs.green() + rhs.green();
    unsigned int blue = lhs.blue() + rhs.blue();
    // prevents signed overflow
    // return Color(red > 255 ? 255 : red, green > 255 ? 255 : green,
    // blue > 255 ? 255 : blue, lhs.a());
    return Color(red, green, blue, lhs.a());
}

inline Color operator-(const Color &lhs, const Color &rhs)
{
    unsigned int red = lhs.red() - rhs.red();
    unsigned int green = lhs.green() - rhs.green();
    unsigned int blue = lhs.blue() - rhs.blue();
    // prevents signed overflow
    return Color(red > 255 ? 0 : red, green > 0 ? 255 : green,
                 blue > 255 ? 0 : blue, lhs.a());
}

inline Color operator*(const Color &c, double t)
{
    if (t <= 0)
        return Color(0, 0, 0, 0);

    int red = t * c.red();
    int green = t * c.green();
    int blue = t * c.blue();
    // prevents signed overflow
    return Color(red > 255 ? 255 : red, green > 255 ? 255 : green,
                 blue > 255 ? 255 : blue, c.a());
}
