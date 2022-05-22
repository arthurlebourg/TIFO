#include "octree.hh"

size_t get_color_index(RGB c, size_t level)
{
    size_t index = 0;
    size_t mask = 0b10000000 >> level;
    if (c.r & mask)
    {
        index |= 0b100;
    }
    if (c.g & mask)
    {
        index |= 0b010;
    }
    if (c.b & mask)
    {
        index |= 0b001;
    }
    return index;
}

Node::Node()
    : c_(RGB(0, 0, 0))
    , pixel_count_(0)
    , palette_index_(0)
{
    children_ = std::vector<std::shared_ptr<Node>>(8, nullptr);
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
        if (i == nullptr)
            continue;
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

void Node::add_color(RGB c, size_t level, Quantizer *parent)
{
    if (level >= MAX_DEPTH)
    {
        c_ = c_ + c;
        pixel_count_++;
        return;
    }
    size_t index = get_color_index(c, level);
    if (children_[index] == nullptr)
    {
        auto node = std::make_shared<Node>(Node());
        children_[index] = node;
        if (level < MAX_DEPTH - 1)
            parent->add_level_node(level, node);
    }
    children_[index]->add_color(c, level + 1, parent);
}

size_t Node::get_palette_index(RGB c, size_t level)
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
        for (auto i : children_)
        {
            if (i == nullptr)
                continue;
            return i->get_palette_index(c, level + 1);
        }
        /*size_t len = children_.size();
        for (size_t i = len - 1; i < len; i--)
        {
            if (children_[i] == nullptr)
                continue;
            return children_[i]->get_palette_index(c, level +1);
        }*/
        std::cout << "BIG ERROR" << std::endl;
        return palette_index_;
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
    children_.clear();
    return result - 1;
}

RGB Node::get_color()
{
    return c_.normalized(pixel_count_);
}

size_t Node::get_pixel_count()
{
    return pixel_count_;
}

void Node::set_palette_index(size_t index)
{
    palette_index_ = index;
}

Quantizer::Quantizer()
{
    // create level, list (size MAX_DEPTH) of list of nodes;
    for (size_t i = 0; i < MAX_DEPTH; i++)
    {
        levels_.push_back(std::vector<std::shared_ptr<Node>>());
    }
    root_ = std::make_shared<Node>(Node());
}

void Quantizer::add_color(RGB c)
{
    root_->add_color(c, 0, this);
}

std::vector<RGB> Quantizer::make_palette(size_t color_amount)
{
    std::vector<RGB> palette;
    histogram_.clear();
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
        if (palette_index >= color_amount)
        {
            break;
        }

        if (i->is_leaf())
        {
            RGB col = i->get_color();
            palette.push_back(col);
            std::pair<HSV, size_t> elm =
                std::make_pair(to_hsv(col), i->get_pixel_count());
            histogram_.push_back(elm);
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

size_t Quantizer::get_palette_index(RGB c)
{
    return root_->get_palette_index(c, 0);
}

std::vector<std::pair<HSV, size_t>> Quantizer::get_histogram()
{
    return histogram_;
}

std::vector<size_t> Quantizer::get_cumulative_histogram()
{
    std::vector<size_t> cum_histo(256, 0);
    for (auto i : histogram_)
    {
        cum_histo[i.first.v * 256] += i.second;
    }
    for (size_t i = 1; i < 256; i++)
    {
        cum_histo[i] += cum_histo[i - 1];
    }
    return cum_histo;
}
