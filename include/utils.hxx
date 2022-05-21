#pragma once

#include "utils.hh"

template <typename T, typename G>
T saturate_cast(G value, T min, T max)
{
    G min_g = min;
    G max_g = max;
    if (value < min_g)
        return min;
    if (value > max_g)
        return max;
    return (T)value;
}
