#pragma once


#include "algorithm/settings.hpp"
#include "algorithm/algorithm_depth_map.hpp"
#include "algorithm/algorithm_roi.hpp"
#include "algorithm/algorithm_voxels.hpp"
#include "algorithm/algorithm_surface.hpp"
#include "algorithm/debug_utils.hpp"


#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/trigonometric.hpp"


#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <span>
#include <iostream>



/**
 * returns tuple: (roi spheres, indexes of around molexule spheres, triangular surface)
 */
template<size_t E1, size_t E2>
std::tuple<std::vector<glm::vec4>, std::unordered_set<size_t>, std::vector<glm::vec3>> compute(
    std::span<glm::vec4, E1> tunnel_spheres,
    std::span<glm::vec4, E2> molecule_spheres,
    const AlgorithmSettings& settings, // [todo] maybe pass by value
    bool debug_info = false
);



template<size_t E1, size_t E2>
std::tuple<std::vector<glm::vec4>, std::unordered_set<size_t>, std::vector<glm::vec3>> compute(
    std::span<glm::vec4, E1> tunnel_spheres,
    std::span<glm::vec4, E2> molecule_spheres,
    const AlgorithmSettings& settings,
    bool debug_info
) {
    if (debug_info) {
        std::cout << "[debug] compute start\n";
        std::cout << "[debug] tunnel spheres count : " << tunnel_spheres.size() << "\n";
        std::cout << "[debug] molecule spheres count : " << molecule_spheres.size() << "\n";
    }

    adt::time_point debug_t_start = adt::now();

    std::optional<float> depth_map_probe_o = settings.depth_map_use_probe ? std::make_optional(settings.probe) : std::nullopt;

    std::unordered_map<size_t, std::pair<size_t, float>> around_m_spheres = compute_around_m_spheres(tunnel_spheres, molecule_spheres, settings.healpix_nside, depth_map_probe_o, settings.depth_map_filter, debug_info);

    adt::time_point debug_t0 = adt::now();
    std::vector<glm::vec4> roi_spheres = compute_roi(around_m_spheres, tunnel_spheres, molecule_spheres);
    std::unordered_set<size_t> around_m_spheres_indexes = compute_around_m_spheres_indexes(around_m_spheres);

    if (debug_info) {
        std::cout << "[debug] roi spheres count : " << roi_spheres.size() << "\n";
        std::cout << "[debug] roi + around atoms\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(adt::now() - debug_t0);
        std::cout << "\n";
    }

    auto [ voxels, voxels_start ] = compute_voxels(tunnel_spheres, molecule_spheres, std::span(roi_spheres), settings.voxel_size, settings.probe, settings.molecule_spheres_only_intersecting, settings.segmentation, settings.force_tunnel, debug_info);

    std::vector<glm::vec3> surface = compute_surface(voxels, voxels_start, settings.voxel_size, settings.smoothing_iter_count, debug_info);

    adt::time_point debug_t_end = adt::now();
    if (debug_info) {
        std::cout << "[debug] triangles count : " << surface.size() / 6 << "\n";
        std::cout << "[debug] compute end\n";
        std::cout << "[debug] total time : ";
        adt::duration_print_as_ns(debug_t_end - debug_t_start);
        std::cout << "\n\n";
    }

    return { std::move(roi_spheres), std::move(around_m_spheres_indexes), std::move(surface) };
}