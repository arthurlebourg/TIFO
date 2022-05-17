/*#include "octree.hh"

void Octree::insert_color(Color c)
{
    auto indexes = generateColorIndex(c);
    Octree cur = *this;
    for (int i = 0; i < 8; i++)
    {
        const auto idx = indexes[i];
        auto &child = cur.children_[idx];
        uint8_t ib = shrinkColor(c.blue(), i),
                ig = shrinkColor(c.green(), i),
                ir = shrinkColor(c.red(), i);
        if (child == nullptr)
        {
            child = Octree(Color(ib, ig, ir));
            nodes[i].push_back(child);
        }
        cur = child;
    }
}

uint8_t Octree::shrinkColor(uint8_t color, int remain) {
    return color & (uint8_t) (UCHAR_MAX << (8 - remain - 1));
}
*/
