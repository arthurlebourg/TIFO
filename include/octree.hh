#pragma once

#include <climits>
#include <iostream>
#include <vector>

#include "color.hh"

class Octree
{
public:
    Octree(Color c)
        : c_(c)
    {}

    Color color() const
    {
        return c_;
    }

    void insert_color(Color c);

    void generatePalette();

    uint8_t shrinkColor(uint8_t color, int remain);

private:
    Color c_;
    std::vector<Octree> children_;
};
