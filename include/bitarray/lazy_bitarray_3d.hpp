#pragma once


#include "bitarray/bitarray_3d.hpp"


#include "glm/gtx/std_based_type.hpp"


#include <vector>
#include <span>
#include <variant>
#include <functional>



enum class plop_type { none, reduce, expand };



// [todo] is this correct usage
/**
 * struct containing static functions used for computing boundary points
 * defined in "boundary_points.cpp"
 */
struct lazy_bitarray_3d_boundary_points_wrapper;

class lazy_bitarray_3d_node;

class lazy_bitarray_3d_node_array
{
    /* sizing */
    glm::size3_t size;
    std::span<glm::size3_t> layers_sizing;

    glm::size3_t child_size;
    glm::size3_t child_count;
    size_t child_count_1d;
    glm::size3_t last_child_size;

    /* all bits same counters */
    size_t all_zero_count;
    size_t all_one_count;

    /* nodes */
    std::vector<lazy_bitarray_3d_node> nodes;

public:
    lazy_bitarray_3d_node_array(glm::size3_t size, std::span<glm::size3_t> layers_sizing_, bool value);

    bool get_bit(glm::size3_t index) const;
    bool set_bit(glm::size3_t index, bool value); /* returns true if ploping is possible after set_bit */

    bool index_valid(glm::size3_t index) const;
    void index_valid_or_throw(glm::size3_t index) const;

    friend lazy_bitarray_3d_boundary_points_wrapper;
};

class lazy_bitarray_3d_node
{
    std::variant<bool, lazy_bitarray_3d_node_array, bitarray_3d> data;

public:
    lazy_bitarray_3d_node(bool value);

    bool get_bit(glm::size3_t index) const;
    plop_type set_bit_without_expand(glm::size3_t index, bool value);
    void set_bit_expand(glm::size3_t size, std::span<glm::size3_t> layers_sizing, glm::size3_t index, bool value);

    friend lazy_bitarray_3d_boundary_points_wrapper;
};

class lazy_bitarray_3d
{
    glm::size3_t size;
    std::vector<glm::size3_t> layers_sizing_v;
    std::span<glm::size3_t> layers_sizing;

    lazy_bitarray_3d_node data;

public:
    lazy_bitarray_3d(std::vector<glm::size3_t> layers_sizing_, bool value = false);

    bool get_bit(glm::size3_t index) const;
    void set_bit(glm::size3_t index, bool value);

    glm::size3_t get_size() const;

    friend lazy_bitarray_3d_boundary_points_wrapper;
};