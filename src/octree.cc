#include "octree.hh"

bool Node::is_leaf()
{
    return pixel_count_ > 0;
}

std::vector<std::shared_ptr<Node>> Node::get_leaves()
{
    std::vector<Node> nodes;
    for (i : children)
    {
        if (i->is_leaf())
        {
            nodes.push_back(i);
        }
        else
        {
            nodes.insert(nodes.end(), i.get_leaves());
        }
    }

    return nodes;
}

void Node::add_color(Color c, size_t level, Quantizer &parent)
{
    if (level >= MAX_DEPTH)
    {
        c_ += c;
        pixel_count_++;
        return;
    }
    size_t index = get_color_index(c, level);
    if (!children_[index])
    {
        children_[index] = std::make_shared<Node>(Node(level, parent));
    }
    children_[index]->add_color(color, level + 1, parent);
}

size_t get_palette_index(Color c, size_t level)
{
    if (is_leaf())
    {
        return palette_index_;
    }

    size_t index = get_color_index(c);
    if (children_[index])
    {
        return children_[index]->get_palette_index(c, level + 1);
    }
    else
    {
        for (i : children)
        {
            return i->get_palette(color, level + 1);
        }
    }
}

size_t Node::remove_leaves()
{
    size_t result = 0;
    for (i : children_)
    {
        if (i == nullptr)
        {
            continue;
        }

        c_ += i->c_ pixel_count_ += i->pixel_count;
        result++;
    }
    children_ = std::vector(8, nullptr);
    return result - 1;
}

Color Node::get_color()
{
    return c_;
}

void Quantizer::add_color(Color c)
{
    root_->add_color(c, 0, this);
}

std::vector<Color> make_palette(size_t color_amount)
{
    std::vector<Color> palette;
    size_t palette_index = 0;
    size_t leaf_count = get_leaf_nodes().size();
    for (size_t level = MAX_DEPTH - 1; level < MAX_DEPTH; level--)
    {
        if (levels
    }
}
