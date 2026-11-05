#include "union_find/union_find.hpp"



uint16_t union_find::new_index()
{
    uint16_t index = static_cast<uint16_t>(forest.size() / 2);
    forest.push_back(index);
    forest.push_back(0);

    return index;
}

uint16_t union_find::find(uint16_t index)
{
    uint16_t root = index;

    while (true) {
        uint16_t parent = forest[root * 2];
        if (parent == root) {
            break;
        }
        root = parent;
    }

    while (true) {
        uint16_t& parent_ref = forest[index * 2];
        uint16_t parent = parent_ref;
        
        if (parent == root) {
            break;
        }

        parent_ref = root;
        index = parent;
    }

    return root;
}

uint16_t union_find::merge(uint16_t index_a, uint16_t index_b)
{
    index_a = find(index_a);
    index_b = find(index_b);

    if (index_a == index_b) {
        return index_a;
    }

    uint16_t rank_a = forest[index_a * 2 + 1];
    uint16_t rank_b = forest[index_b * 2 + 1];

    if (rank_a < rank_b) {
        std::swap(index_a, index_b);
        std::swap(rank_a, rank_b);
    }

    forest[index_b * 2] = index_a;

    if (rank_a == rank_b) {
        forest[index_a * 2 + 1] += 1;
    }

    return index_a;
}

size_t union_find::size() const
{
    return forest.size() / 2;
}