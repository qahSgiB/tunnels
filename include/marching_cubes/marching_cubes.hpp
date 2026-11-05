#pragma once



#include "glm/vec3.hpp"
#include "glm/gtx/std_based_type.hpp"


#include <vector>
#include <unordered_map>
#include <utility>
#include <cstdint>



class marching_cubes
{
    glm::vec3 offset;
    float cube_size;

    glm::size3_t cubes_count;

    std::vector<size_t> faces;
    std::unordered_map<size_t, std::pair<glm::vec3, glm::vec3>> vertices;

public:
    marching_cubes(glm::vec3 offset_, float cube_size_, glm::size3_t cubes_count_);

    void add_cube(glm::size3_t index, uint8_t config);

    std::vector<glm::vec3> get_triangles(size_t smoothing_iter_count = 0, bool debug_info = false) const;

private:
    size_t edge_get_vertex_index(glm::size3_t index, uint8_t edge_index) const;
};