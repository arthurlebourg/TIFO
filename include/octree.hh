#pragma once

#include <climits>
#include <iostream>
#include <vector>

#include "color.hh"

#define MAX_DEPTH 16

class Quantizer;
class Node;

size_t get_color_index(Color c, size_t level)
{
    size_t index = 0;
    size_t mask = 0b10000000 >> level;
    if (color.red() & mask)
    {
        index |= 0b100;
    }
    if (color.green() & mask)
    {
        index |= 0b010;
    }
    if (color.blue() & mask)
    {
        index |= 0b001;
    }
    return index;
}

class Node
{
public:
    Node(size_t level, Quantizer &parent)
        : c_(Color(0, 0, 0))
        , pixel_count_(0)
        , palette_index_(0)
    {
        children_ = std::vector(8, nullptr);
        if (level < MAX_DEPTH)
        {
            parent.add_level_node(level, this);
        }
    }

    bool is_leaf();

    std::vector<std::shared_ptr<Node>> get_leaves();

    void add_color(Color c, size_t level, Quantizer &parent);

    size_t get_palette_index(Color c, size_t level);

    size_t remove_leaves();

    Color get_color();

private:
    Color c_;
    size_t pixe_count_;
    size_t palette_index_;
    std::vector<std::shared_ptr<Node>> children_;
}

class Quantizer
{
public:
    Quantizer()
        : root_(std::make_shared<Node>(Node(0, this)))
    {
        // create level, list (size MAX_DEPTH) of list of nodes;
    }

    void add_color(Color c);

    std::vector<Color> make_palette(size_t color_count);

    std::vector<std::shared_ptr<Node>> get_leaf_nodes();

    void add_level_node(size_t level, std::shared_ptr<Node> node);

    size_t get_palette_index(Color c);

private:
    std::vector<std::vector<std::shared_ptr<Node>>> levels_;
    std::shared_ptr<Node> root_;
}
