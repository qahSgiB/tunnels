#pragma once



#include "bitarray/lazy_bitarray_3d.hpp"


#include "glm/gtx/std_based_type.hpp"


#include <vector>



std::vector<glm::size3_t> lazy_bitarray_3d_boundary_points(const lazy_bitarray_3d& lazy_bitarray);