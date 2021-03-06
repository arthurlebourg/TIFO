#pragma once

#include <climits>
#include <iostream>
#include <memory>
#include <vector>

#include "color.hh"

#define MAX_DEPTH 8

class Quantizer;
class Node;

size_t get_color_index(RGB c, size_t level);

class Node
{
public:
    Node();

    bool is_leaf();

    std::vector<std::shared_ptr<Node>> get_leaves();

    void add_color(RGB c, size_t level, Quantizer *parent);

    size_t get_palette_index(RGB c, size_t level);

    size_t remove_leaves();

    RGB get_color();

    size_t get_pixel_count();

    void set_palette_index(size_t index);

private:
    RGB c_;
    size_t pixel_count_;
    size_t palette_index_;
    std::vector<std::shared_ptr<Node>> children_;
};

class Quantizer
{
public:
    Quantizer();

    void add_color(RGB c);

    std::vector<RGB> make_palette(size_t color_count);

    std::vector<std::shared_ptr<Node>> get_leaf_nodes();

    void add_level_node(size_t level, std::shared_ptr<Node> node);

    size_t get_palette_index(RGB c);

    std::vector<std::pair<HSV, size_t>> get_histogram();

    std::vector<size_t> get_lightness_cumulative_histogram();

private:
    std::vector<std::vector<std::shared_ptr<Node>>> levels_;
    std::shared_ptr<Node> root_;
    std::vector<std::pair<HSV, size_t>> histogram_;
};
