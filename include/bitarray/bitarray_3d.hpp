#pragma once


#include "glm/gtx/std_based_type.hpp"


#include <vector>
#include <utility>



// [todo] is this correct usage
/**
 * struct containing static functions used for computing boundary points
 * defined in "boundary_points.cpp"
 */
struct lazy_bitarray_3d_boundary_points_wrapper;

class bitarray_3d
{
    std::vector<uint64_t> bits;

    glm::size3_t size;
    size_t size_1d;

    size_t one_count;


public:
    bitarray_3d(glm::size3_t size_, bool value = false);

    bool get_bit(glm::size3_t index) const;
    bool set_bit(glm::size3_t index, bool value); /* returns true if ploping is possible after set_bit */

    glm::size3_t get_size() const;
    size_t get_size_1d() const;

private:
    bool index_valid(glm::size3_t index) const;
    void index_valid_or_error(glm::size3_t index, const char* message) const;

    bool get_bit_unsafe(glm::size3_t index) const;

    std::pair<size_t, size_t> index_to_bit_index(glm::size3_t index) const;


    friend lazy_bitarray_3d_boundary_points_wrapper;
};