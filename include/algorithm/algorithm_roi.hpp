#pragma once


#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/geometric.hpp"


#include <vector>
#include <unordered_map>
#include <span>
#include <utility>
#include <ranges>



template<size_t E1, size_t E2>
std::vector<glm::vec4> compute_roi(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres
);



template<size_t E1, size_t E2>
std::vector<glm::vec4> compute_roi(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres
) {
    auto rng = around_m_spheres | std::views::transform([t_spheres, m_spheres] (auto around_m_spher_iter) {
        glm::vec4 t_sphere = t_spheres[around_m_spher_iter.second.first];
        glm::vec4 m_sphere = m_spheres[around_m_spher_iter.first];

        glm::vec3 center = (t_sphere + m_sphere) / 2.0f;
        float radius = glm::distance(t_sphere, m_sphere) / 2.0f;

        return glm::vec4(center, radius);
    });

    return std::vector<glm::vec4>(rng.begin(), rng.end());
}