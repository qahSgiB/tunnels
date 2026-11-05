#pragma once


#include "algorithm/debug_utils.hpp"

#include "bitarray/lazy_bitarray_3d.hpp"
#include "bitarray/boundary_points.hpp"

#include "marching_cubes/marching_cubes.hpp"


#include "glm/vec3.hpp"
#include "glm/gtx/std_based_type.hpp"
#include "glm/gtx/hash.hpp"


#include <vector>
#include <unordered_set>
#include <cstdint>
#include <iostream>



inline std::vector<glm::vec3> compute_surface(
    const lazy_bitarray_3d& voxels,
    glm::vec3 voxels_start,
    float voxel_size,
    size_t smoothing_iter_count,
    bool debug_info = false
);



std::vector<glm::vec3> compute_surface(
    const lazy_bitarray_3d& voxels,
    glm::vec3 voxels_start,
    float voxel_size,
    size_t smoothing_iter_count,
    bool debug_info
) {
    adt::time_point debug_t0 = adt::now();
    
    std::vector<glm::size3_t> boundary_points = lazy_bitarray_3d_boundary_points(voxels);

    std::unordered_set<glm::size3_t> marching_cubes_points;

    for (glm::size3_t point : boundary_points) {
        marching_cubes_points.emplace(point.x    , point.y    , point.z    );
        marching_cubes_points.emplace(point.x + 1, point.y    , point.z    );
        marching_cubes_points.emplace(point.x    , point.y + 1, point.z    );
        marching_cubes_points.emplace(point.x + 1, point.y + 1, point.z    );
        marching_cubes_points.emplace(point.x    , point.y    , point.z + 1);
        marching_cubes_points.emplace(point.x + 1, point.y    , point.z + 1);
        marching_cubes_points.emplace(point.x    , point.y + 1, point.z + 1);
        marching_cubes_points.emplace(point.x + 1, point.y + 1, point.z + 1);
    }

    adt::time_point debug_t1 = adt::now();
    if (debug_info) {
        std::cout << "[debug] surface boundary point\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t1 - debug_t0);
        std::cout << "\n";
    }

    glm::size3_t voxel_count = voxels.get_size();

    marching_cubes mc(voxels_start - voxel_size / 2.0f, voxel_size, voxel_count + static_cast<size_t>(1));

    for (glm::size3_t point : marching_cubes_points) {
        bool v000 = true;
        bool v001 = true;
        bool v010 = true;
        bool v011 = true;
        bool v100 = true;
        bool v101 = true;
        bool v110 = true;
        bool v111 = true;

        if (point.x == 0) {
            v000 = false;
            v010 = false;
            v100 = false;
            v110 = false;
        } else if (point.x == voxel_count.x) {
            v001 = false;
            v011 = false;
            v101 = false;
            v111 = false;
        }

        if (point.y == 0) {
            v000 = false;
            v001 = false;
            v100 = false;
            v101 = false;
        } else if (point.y == voxel_count.y) {
            v010 = false;
            v011 = false;
            v110 = false;
            v111 = false;
        }

        if (point.z == 0) {
            v000 = false;
            v001 = false;
            v010 = false;
            v011 = false;
        } else if (point.z == voxel_count.z) {
            v100 = false;
            v101 = false;
            v110 = false;
            v111 = false;
        }

        if (v000) { v000 = voxels.get_bit(point - glm::size3_t(1, 1, 1)); }
        if (v001) { v001 = voxels.get_bit(point - glm::size3_t(0, 1, 1)); }
        if (v010) { v010 = voxels.get_bit(point - glm::size3_t(1, 0, 1)); }
        if (v011) { v011 = voxels.get_bit(point - glm::size3_t(0, 0, 1)); }
        if (v100) { v100 = voxels.get_bit(point - glm::size3_t(1, 1, 0)); }
        if (v101) { v101 = voxels.get_bit(point - glm::size3_t(0, 1, 0)); }
        if (v110) { v110 = voxels.get_bit(point - glm::size3_t(1, 0, 0)); }
        if (v111) { v111 = voxels.get_bit(point - glm::size3_t(0, 0, 0)); }

        uint8_t cube_config = (static_cast<uint8_t>(v000) << 0) | (static_cast<uint8_t>(v001) << 1) | (static_cast<uint8_t>(v011) << 2) | (static_cast<uint8_t>(v010) << 3) | (static_cast<uint8_t>(v100) << 4) | (static_cast<uint8_t>(v101) << 5) | (static_cast<uint8_t>(v111) << 6) | (static_cast<uint8_t>(v110) << 7);

        mc.add_cube(point, cube_config);
    }

    adt::time_point debug_t2 = adt::now();
    if (debug_info) {
        std::cout << "[debug] surface add cubes\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t2 - debug_t1);
        std::cout << "\n";
    }

    return mc.get_triangles(smoothing_iter_count, debug_info);
}