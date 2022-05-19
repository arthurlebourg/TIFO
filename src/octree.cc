#include "octree.hh"

size_t get_color_index(Color c, size_t level)
{
    size_t index = 0;
    size_t mask = 0b10000000 >> level;
    if (c.red() & mask)
    {
        index |= 0b100;
    }
    if (c.green() & mask)
    {
        index |= 0b010;
    }
    if (c.blue() & mask)
    {
        index |= 0b001;
    }
    return index;
}

Node::Node(size_t level, Quantizer *parent)
    : c_(Color(0, 0, 0, 0))
    , pixel_count_(0)
    , palette_index_(0)
{
    children_ = std::vector<std::shared_ptr<Node>>(8, nullptr);
    if (level < MAX_DEPTH)
    {
        parent->add_level_node(level, std::make_shared<Node>(*this));
    }
}
bool Node::is_leaf()
{
    return pixel_count_ > 0;
}

std::vector<std::shared_ptr<Node>> Node::get_leaves()
{
    std::vector<std::shared_ptr<Node>> nodes;
    for (auto i : children_)
    {
        if (i->is_leaf())
        {
            nodes.push_back(i);
        }
        else
        {
            for (auto j : i->get_leaves())
                nodes.push_back(j);
        }
    }

    return nodes;
}

void Node::add_color(Color c, size_t level, Quantizer *parent)
{
    if (level >= MAX_DEPTH)
    {
        c_ = c_ + c;
        pixel_count_++;
        return;
    }
    size_t index = get_color_index(c, level);
    if (!children_[index])
    {
        children_[index] = std::make_shared<Node>(Node(level, parent));
    }
    children_[index]->add_color(c, level + 1, parent);
}

size_t Node::get_palette_index(Color c, size_t level)
{
    if (is_leaf())
    {
        return palette_index_;
    }

    size_t index = get_color_index(c, level);
    if (children_[index])
    {
        return children_[index]->get_palette_index(c, level + 1);
    }
    else
    {
        return children_[0]->get_palette_index(c, level + 1);
    }
}

size_t Node::remove_leaves()
{
    size_t result = 0;
    for (auto i : children_)
    {
        if (i == nullptr)
        {
            continue;
        }

        c_ = c_ + i->c_;
        pixel_count_ += i->pixel_count_;
        result++;
    }
    children_ = std::vector<std::shared_ptr<Node>>(8, nullptr);
    return result - 1;
}

Color Node::get_color()
{
    return c_;
}

void Node::set_palette_index(size_t index)
{
    palette_index_ = index;
}

void Quantizer::add_color(Color c)
{
    root_->add_color(c, 0, this);
}

std::vector<Color> Quantizer::make_palette(size_t color_amount)
{
    std::vector<Color> palette;
    size_t palette_index = 0;
    size_t leaf_count = get_leaf_nodes().size();
    for (size_t level = MAX_DEPTH - 1; level < MAX_DEPTH; level--)
    {
        if (levels_[level].size() > 0)
        {
            for (auto i : levels_[level])
            {
                leaf_count -= i->remove_leaves();
                if (leaf_count <= color_amount)
                    break;
            }
            if (leaf_count <= color_amount)
                break;
            levels_[level].clear();
        }
    }
    for (auto i : get_leaf_nodes())
    {
        if (palette_index > color_amount)
        {
            break;
        }

        if (i->is_leaf())
        {
            palette.push_back(i->get_color());
        }
        i->set_palette_index(palette_index);
        palette_index++;
    }
    return palette;
}

std::vector<std::shared_ptr<Node>> Quantizer::get_leaf_nodes()
{
    return root_->get_leaves();
}

void Quantizer::add_level_node(size_t level, std::shared_ptr<Node> node)
{
    levels_[level].push_back(node);
}

size_t Quantizer::get_palette_index(Color c)
{
    return root_->get_palette_index(c, 0);
}
