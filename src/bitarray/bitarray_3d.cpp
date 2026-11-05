#include "bitarray/bitarray_3d.hpp"



#include "bitarray/bitarray_3d.hpp"



#include <string>
#include <stdexcept>



bitarray_3d::bitarray_3d(glm::size3_t size_, bool value)
:
    size(size_),
    size_1d(size.x * size.y * size.z),
    one_count(value ? size_1d : 0)
{
    if (size_1d != 0) {
        bits = std::vector<uint64_t>((size_1d - 1) / 64 + 1, value ? ~static_cast<uint64_t>(0) : static_cast<uint64_t>(0));
    }
}

bool bitarray_3d::get_bit(glm::size3_t index) const
{
    index_valid_or_error(index, "bitarray_3d::get_bit : index out of range");
    return get_bit_unsafe(index);
}

bool bitarray_3d::set_bit(glm::size3_t index, bool value)
{
    index_valid_or_error(index, "bitarray_3d::set_bit : index out of range");

    auto [ v_index, b_index ] = index_to_bit_index(index);

    if (value) {
        uint64_t& b = bits[v_index];

        if ((b >> b_index & 0b1) == 1) {
            return false;
        }
        
        b |= static_cast<uint64_t>(1) << b_index;
        one_count++;

        return one_count == size_1d;
    } else {
        uint64_t& b = bits[v_index];

        if ((b >> b_index & 0b1) == 0) {
            return false;
        }

        b &= ~(static_cast<uint64_t>(1) << b_index);
        one_count--;
        
        return one_count == 0;
    }
}

glm::size3_t bitarray_3d::get_size() const { return size; }
size_t bitarray_3d::get_size_1d() const { return size_1d; }

bool bitarray_3d::index_valid(glm::size3_t index) const
{
    return index.x < size.x && index.y < size.y && index.z < size.z;
}

void bitarray_3d::index_valid_or_error(glm::size3_t index, const char* message) const
{
    if (!index_valid(index)) {
        throw std::out_of_range(message);
    }
}

bool bitarray_3d::get_bit_unsafe(glm::size3_t index) const
{
    auto [ v_index, b_index ] = index_to_bit_index(index);
    return ((bits[v_index] >> b_index) & 0b1) == 1;
}

std::pair<size_t, size_t> bitarray_3d::index_to_bit_index(glm::size3_t index) const
{
    size_t index_1d = index.z * size.y * size.x + index.y * size.x + index.x;
    return { index_1d / 64, index_1d % 64 };
}