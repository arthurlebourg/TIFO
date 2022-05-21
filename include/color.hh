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

    Color(size_t red, size_t green, size_t blue, size_t a)
        : red_(red)
        , green_(green)
        , blue_(blue)
        , a_(a)
    {}

    size_t red() const
    {
        return red_;
    }

    size_t green() const
    {
        return green_;
    }

    size_t blue() const
    {
        return blue_;
    }

    size_t a() const
    {
        return a_;
    }

    Color normalized(size_t pixel_count)
    {
        std::cout << "red: " << red_ << " green: " << green_
                  << " blue: " << blue_ << " count: " << pixel_count
                  << std::endl;
        Color res(red_ / pixel_count, green_ / pixel_count, blue_ / pixel_count,
                  a_);
        std::cout << "red: " << res.red_ << " green: " << res.green_
                  << " blue: " << res.blue_ << std::endl;
        return res;
    }

    Color normalized_biased(size_t pixel_count)
    {
        Color res(red_ / pixel_count, green_ / pixel_count, blue_ / pixel_count,
                  a_);
        size_t max = 0;
        size_t min = 0;
        if (res.red_ > res.blue_)
        {
            if (res.red_ > res.green_)
            {
                max = res.red_;
            }
            else
            {
                max = res.green_;
            }
            if (res.blue_ > res.green_)
            {
                min = res.green_;
            }
            else
            {
                min = res.blue_;
            }
        }
        else
        {
            if (res.blue_ > res.green_)
            {
                max = res.blue_;
            }
            else
            {
                max = res.green_;
            }
            if (res.red_ > res.green_)
            {
                min = res.green_;
            }
            else
            {
                min = res.red_;
            }
        }

        if (res.red_ > res.blue_)
        {
            if (res.red_ > res.green_)
            {
                res.red_ =
                    max - min + res.red_ > 255 ? 255 : max - min + res.red_;
            }
            else
            {
                res.green_ =
                    max - min + res.green_ > 255 ? 255 : max - min + res.green_;
            }
            if (res.blue_ > res.green_)
            {
                res.green_ -= max - min > res.green_ ? res.green_ : max - min;
            }
            else
            {
                res.blue_ -= max - min > res.blue_ ? res.blue_ : max - min;
            }
        }
        else
        {
            if (res.blue_ > res.green_)
            {
                res.blue_ =
                    max - min + res.blue_ > 255 ? 255 : max - min + res.blue_;
            }
            else
            {
                res.green_ =
                    max - min + res.green_ > 255 ? 255 : max - min + res.green_;
            }
            if (res.red_ > res.green_)
            {
                res.green_ -= max - min > res.green_ ? res.green_ : max - min;
            }
            else
            {
                res.red_ -= max - min > res.red_ ? res.red_ : max - min;
            }
        }
        return res;
    }

private:
    size_t red_;
    size_t green_;
    size_t blue_;
    size_t a_;
};

inline std::ostream &operator<<(std::ostream &os, Color &col)
{
    return os << "(R: " << col.red() << ", G: " << col.green()
              << ", B: " << col.blue() << ", A: " << col.a()
              << ") hex: " << std::hex << col.red() << col.green() << col.blue()
              << std::dec << std::endl;
}

inline Color operator+(const Color &c, size_t t)
{
    size_t red = t + c.red();
    size_t green = t + c.green();
    size_t blue = t + c.blue();
    return Color(red, green, blue, c.a());
}

inline Color operator+(const Color &lhs, const Color &rhs)
{
    size_t red = lhs.red() + rhs.red();
    size_t green = lhs.green() + rhs.green();
    size_t blue = lhs.blue() + rhs.blue();
    return Color(red, green, blue, lhs.a());
}

inline Color operator-(const Color &lhs, const Color &rhs)
{
    size_t red = lhs.red() - rhs.red();
    size_t green = lhs.green() - rhs.green();
    size_t blue = lhs.blue() - rhs.blue();
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
