#pragma once


#include <vector>
#include <cstdint>
#include <cstddef>



class union_find
{
    std::vector<uint16_t> forest;

public:
    uint16_t new_index();

    uint16_t find(uint16_t index);

    uint16_t merge(uint16_t index_a, uint16_t index_b);

    size_t size() const;
};