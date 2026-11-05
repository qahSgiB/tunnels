#include "bitarray/lazy_bitarray_3d.hpp"


#include <utility> 
#include <stdexcept>



/**
 *   helper constants
 * arithmetic operation (+, -, ...) with scalars requeries scalar to have exactly size_t type
 * using numeric literals (0, 1, ...) doesn't work because they have type int
 */
constexpr static size_t st1 = 1;



lazy_bitarray_3d_node_array::lazy_bitarray_3d_node_array(glm::size3_t size, std::span<glm::size3_t> layers_sizing_, bool value)
:
    size(size),
    layers_sizing(layers_sizing_.last(layers_sizing_.size() - 1)),
    child_size(layers_sizing_[0]),
    child_count((size - st1) / child_size + st1),
    child_count_1d(child_count.x * child_count.y * child_count.z),
    last_child_size((size - st1) % child_size + st1),
    all_zero_count(value ? 0 : 1),
    all_one_count(value ? 1 : 0),
    nodes(child_count_1d, lazy_bitarray_3d_node(value))
{}

bool lazy_bitarray_3d_node_array::get_bit(glm::size3_t index) const
{
    index_valid_or_throw(index);

    glm::size3_t child_index = index / child_size;
    glm::size3_t in_child_index = index % child_size;

    size_t child_index_1d = child_index.x + child_index.y * child_count.x + child_index.z * child_count.y * child_count.x;

    return nodes[child_index_1d].get_bit(in_child_index);
}

bool lazy_bitarray_3d_node_array::set_bit(glm::size3_t index, bool value)
{
    index_valid_or_throw(index);

    glm::size3_t child_index = index / child_size;
    glm::size3_t in_child_index = index % child_size;

    size_t child_index_1d = child_index.x + child_index.y * child_count.x + child_index.z * child_count.y * child_count.x;    

    lazy_bitarray_3d_node& node = nodes[child_index_1d];

    plop_type plop = node.set_bit_without_expand(in_child_index, value);

    if (plop == plop_type::reduce) {
        if (value) {
            all_zero_count++;
            return all_zero_count == child_count_1d;
        } else {
            all_one_count++;
            return all_one_count == child_count_1d;
        }
    } else if (plop == plop_type::expand) {
        glm::size3_t plop_chile_size = child_size;
        if (child_index.x == child_count.x - 1) { plop_chile_size.x = last_child_size.x; }
        if (child_index.y == child_count.y - 1) { plop_chile_size.y = last_child_size.y; }
        if (child_index.z == child_count.z - 1) { plop_chile_size.z = last_child_size.z; }

        node.set_bit_expand(plop_chile_size, layers_sizing, in_child_index, value);

        if (value) {
            all_one_count--;
        } else {
            all_zero_count--;
        }
    }

    return false;
}

bool lazy_bitarray_3d_node_array::index_valid(glm::size3_t index) const
{
    return index.x < size.x && index.y < size.y && index.z < size.z;
}

void lazy_bitarray_3d_node_array::index_valid_or_throw(glm::size3_t index) const
{
    if (!index_valid(index)) {
        throw std::out_of_range("lazy_bitarray_3d_node_array::??? : index out of range");
    }
}



lazy_bitarray_3d_node::lazy_bitarray_3d_node(bool value)
:
    data(std::in_place_index<0>, value)
{}

bool lazy_bitarray_3d_node::get_bit(glm::size3_t index) const
{
    size_t variant_index = data.index();

    if (variant_index == 0) {
        return std::get<0>(data);
    } else if (variant_index == 1) {
        return std::get<1>(data).get_bit(index);
    } else {
        return std::get<2>(data).get_bit(index);
    }
}

plop_type lazy_bitarray_3d_node::set_bit_without_expand(glm::size3_t index, bool value)
{
    size_t variant_index = data.index();

    if (variant_index == 0) {
        if (value == std::get<0>(data)) {
            return plop_type::none;
        } else if (value) {
            return plop_type::expand;
        } else {
            return plop_type::expand;
        }
    } else {
        bool plop = (variant_index == 1) ? std::get<1>(data).set_bit(index, value) : std::get<2>(data).set_bit(index, value);

        if (plop) {
            data.emplace<0>(value);
            return plop_type::reduce;
        } else {
            return plop_type::none;
        }
    }
}

void lazy_bitarray_3d_node::set_bit_expand(glm::size3_t size, std::span<glm::size3_t> layers_sizing, glm::size3_t index, bool value)
{
    if (layers_sizing.size() == 0) {
        data.emplace<2>(size, !value);
        std::get<2>(data).set_bit(index, value);
    } else {
        data.emplace<1>(size, layers_sizing, !value);
        std::get<1>(data).set_bit(index, value);
    }
}

lazy_bitarray_3d::lazy_bitarray_3d(std::vector<glm::size3_t> layers_sizing_, bool value)
:
    size(layers_sizing_[0]),
    layers_sizing_v(std::move(layers_sizing_)),
    layers_sizing(std::span(layers_sizing_v).last(layers_sizing_v.size() - 1)),
    data(value)
{}

bool lazy_bitarray_3d::get_bit(glm::size3_t index) const
{
    return data.get_bit(index);
}

void lazy_bitarray_3d::set_bit(glm::size3_t index, bool value)
{
    plop_type plop = data.set_bit_without_expand(index, value);

    if (plop == plop_type::expand) {
        data.set_bit_expand(size, layers_sizing, index, value);
    }
}

glm::size3_t lazy_bitarray_3d::get_size() const { return size; }