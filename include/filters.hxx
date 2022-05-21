#pragma once

#include <iostream>
#include <tbb/parallel_for.h>

#include "filters.hh"

template <typename T>
void median_filter(Matrix<T> &input, Matrix<T> &output, size_t window_size)
{
    size_t half_size = window_size / 2;

    size_t m_rows = input.get_rows();
    size_t m_cols = input.get_cols();

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, m_rows),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++)
            {
                std::vector<T> entries;
                entries.reserve(window_size * window_size);

                for (size_t j = 0; j < m_cols; j++)
                {
                    entries.clear();

                    for (size_t m = 0; m < window_size; m++)
                    {
                        for (size_t n = 0; n < window_size; n++)
                        {
                            size_t ii = i + m - half_size;
                            size_t jj = j + n - half_size;

                            if (ii < m_rows && jj < m_cols)
                                entries.push_back(
                                    input.get_data()[ii * m_cols + jj]);
                        }
                    }

                    std::sort(entries.begin(), entries.end());
                    output.set_value(j, i, entries[entries.size() / 2]);
                }
            }
        });
}
