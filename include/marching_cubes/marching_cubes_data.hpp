#pragma once



#include "glm/vec3.hpp"
#include "glm/gtx/std_based_type.hpp"


#include <array>



namespace marching_cubes_data {
    extern const std::array<glm::vec3, 12> edge_mid_vertex;

    extern const std::array<glm::size3_t, 12> edge_index_add;

    extern const std::array<std::array<uint8_t, 15>, 256> cube_configs;
}