#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

template <typename T, typename G>
T saturate_cast(G value, T min, T max)
{
    G min_g = min;
    G max_g = max;
    if (value < min_g)
    {
        return min;
    }
    else if (value > max_g)
    {
        return max;
    }
    return (T)value;
}

template <typename T>
bool isClose(T x, T y)
{
    return (std::abs(x - y) <= (1e-08 + 1e-05 * std::abs(y)));
}

template <typename T>
std::vector<size_t> keep_best_coords(std::vector<T> v, int max_keypoints,
                                     size_t cols)
{
    std::vector<size_t> index(v.size());
    std::iota(index.begin(), index.end(), 0);
    std::sort(index.begin(), index.end(),
              [v](size_t i1, size_t i2) { return v[i1] > v[i2]; });

    index.resize(max_keypoints);

    std::ofstream coordsFile("cpu_coords.txt");
    for (size_t i = 0; i < index.size(); i++)
    {
        if (v[index[i]] == 0)
            break;
        size_t row_ind = index[i] / cols;
        size_t col_ind = index[i] % cols;
        coordsFile << "[" << row_ind << ", " << col_ind << "]\n";
    }
    coordsFile.close();

    return index;
}
