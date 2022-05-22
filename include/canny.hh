#pragma once

#include <iostream>

#include "matrix.hh"

enum Edge : uint8_t
{
    NONE,
    WEAK,
    STRONG = 255,
};

enum class Blur
{
    NONE,
    GAUSS,
    MEDIAN,
    __LAST_BLUR,
};

inline Blur &operator++(Blur &blur)
{
    return blur = static_cast<Blur>((static_cast<int>(blur) + 1)
                                    % static_cast<int>(Blur::__LAST_BLUR));
}

inline Blur &operator--(Blur &blur)
{
    auto new_blur = static_cast<int>(blur) - 1;
    if (new_blur < 0)
        new_blur = static_cast<int>(Blur::__LAST_BLUR) - 1;
    return blur = static_cast<Blur>(new_blur);
}

inline std::ostream &operator<<(std::ostream &out, Blur blur)
{
    switch (blur)
    {
    case Blur::NONE:
        return out << "NONE";
    case Blur::GAUSS:
        return out << "GAUSS";
    case Blur::MEDIAN:
        return out << "MEDIAN";
    default:
        break;
    }
    return out;
}

void edge_detection(std::vector<Matrix<float>> &buffers, Blur blur,
                    float low_threshold_ratio, float hight_threshold_ratio);
