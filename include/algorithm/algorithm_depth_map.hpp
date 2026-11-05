#pragma once


#include "algorithm/depth_map_entry.hpp"
#include "algorithm/filter.hpp"
#include "algorithm/debug_utils.hpp"


#include "healpix_base.h"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtx/norm.hpp"


#include <ratio>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <span>
#include <utility>
#include <ranges>
#include <limits>
#include <chrono>
#include <iostream>



/**
 * returns mapping: molecule sphere index -> (closest tunnel sphere index, distance)
 */
template<size_t E1, size_t E2>
std::unordered_map<size_t, std::pair<size_t, float>> compute_around_m_spheres(
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres,
    int64_t healpix_nside,
    std::optional<float> depth_map_probe_o,
    DepthMapFilter depth_map_filter,
    bool debug_info = false
);

template<size_t E1, size_t E2>
void compute_t_sphere_depth_map(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres,
    healpix::Healpix_Base2& healpix,
    size_t t_sphere_index,
    float depth_map_probe,
    DepthMapFilter depth_map_filter,
    std::optional<std::pair<adt::duration, adt::duration>>& debug_timer
);

inline void depth_map_project_sphere(
    std::vector<DepthMapEntry>& depth_map,
    healpix::Healpix_Base2& healpix,
    glm::vec3 center,
    size_t sphere_index,
    glm::vec4 sphere,
    bool is_molecule
);

inline std::optional<std::pair<size_t, float>> depth_map_entry_molecule_only(DepthMapEntry entry);

inline void around_m_spheres_update(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    size_t m_sphere_index,
    size_t t_sphere_index,
    float distance
);

inline void add_m_spheres_filter_type_none(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    size_t t_sphere_index
);

inline void add_m_spheres_filter_min_per_pixel(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    healpix::Healpix_Base2& healpix,
    size_t t_sphere_index,
    float filter_angle,
    float filter_distance
);

inline void add_m_spheres_filter_min_per_m_sphere(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4> m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    size_t t_sphere_index,
    glm::vec3 center,
    float filter_angle,
    float filter_distance
);

inline void add_m_spheres_gaussian_min_per_pixel(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    healpix::Healpix_Base2& healpix,
    size_t t_sphere_index,
    float filter_angle
);

inline void add_m_spheres_gaussian_min_per_m_sphere(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4> m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    size_t t_sphere_index,
    glm::vec3 center,
    float filter_angle
);

inline std::unordered_set<size_t> compute_around_m_spheres_indexes(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres
);



template<size_t E1, size_t E2>
std::unordered_map<size_t, std::pair<size_t, float>> compute_around_m_spheres(
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres,
    int64_t healpix_nside,
    std::optional<float> depth_map_probe_o,
    DepthMapFilter depth_map_filter,
    bool debug_info
) {
    std::optional<std::pair<adt::duration, adt::duration>> debug_timer(std::nullopt);

    if (debug_info) {
        debug_timer.emplace(adt::duration::zero(), adt::duration::zero());
    }

    float depth_map_probe = depth_map_probe_o.value_or(0.0f);

    healpix::Healpix_Base2 healpix(healpix_nside, healpix::Healpix_Ordering_Scheme::RING, healpix::SET_NSIDE);

    std::unordered_map<size_t, std::pair<size_t, float>> around_m_spheres;

    for (size_t t_sphere_index = 0; t_sphere_index < t_spheres.size(); t_sphere_index++) {
        compute_t_sphere_depth_map(around_m_spheres, t_spheres, m_spheres, healpix, t_sphere_index, depth_map_probe, depth_map_filter, debug_timer);
    }

    if (debug_info) {
        std::cout << "[debug] depth map projection\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_timer->first);
        std::cout << "\n[debug] depth map filtering\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_timer->second);
        std::cout << "\n";
    }

    return around_m_spheres;
}

template<size_t E1, size_t E2>
void compute_t_sphere_depth_map(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres,
    healpix::Healpix_Base2& healpix,
    size_t t_sphere_index,
    float depth_map_probe,
    DepthMapFilter depth_map_filter,
    std::optional<std::pair<adt::duration, adt::duration>>& debug_timer
) {
    adt::time_point debug_t0 = adt::now();

    glm::vec3 t_sphere(t_spheres[t_sphere_index]);

    std::vector<DepthMapEntry> depth_map(healpix.Npix());

    /* iter tunnel */
    for (size_t other_t_sphere_index = 0; other_t_sphere_index < t_spheres.size(); other_t_sphere_index++) {
        glm::vec4 other_t_sphere = t_spheres[other_t_sphere_index];
        // t_sphere.w += depth_map_probe; // [todo]

        depth_map_project_sphere(depth_map, healpix, glm::vec3(other_t_sphere), other_t_sphere_index, other_t_sphere, false);
    }

    /* iter molecule */
    for (size_t m_sphere_index = 0; m_sphere_index < m_spheres.size(); m_sphere_index++) {
        glm::vec4 m_sphere = m_spheres[m_sphere_index];
        m_sphere.w += depth_map_probe;

        depth_map_project_sphere(depth_map, healpix, t_sphere, m_sphere_index, m_sphere, true);
    }

    adt::time_point debug_t1 = adt::now();

    /* filter + add spheres */
    DepthMapFilterType filter_type = depth_map_filter.type();
    if (filter_type == DepthMapFilterType::None) {
        add_m_spheres_filter_type_none(around_m_spheres, depth_map, t_sphere_index);
    } else if (filter_type == DepthMapFilterType::Min) {
        DepthMapFilterMin filter_min = depth_map_filter.get_min();
        if (filter_min.per_pixel) {
            add_m_spheres_filter_min_per_pixel(around_m_spheres, depth_map, healpix, t_sphere_index, filter_min.angle, filter_min.distance);
        } else {
            add_m_spheres_filter_min_per_m_sphere(around_m_spheres, m_spheres, depth_map, t_sphere_index, t_sphere, filter_min.angle, filter_min.distance);
        }
    } else {
        DepthMapFilterGaussian filter_gaussian = depth_map_filter.get_gaussian();
        if (filter_gaussian.per_pixel) {
            add_m_spheres_gaussian_min_per_pixel(around_m_spheres, depth_map, healpix, t_sphere_index, filter_gaussian.angle);
        } else {
            add_m_spheres_gaussian_min_per_m_sphere(around_m_spheres, m_spheres, depth_map, t_sphere_index, t_sphere, filter_gaussian.angle);
        }
    }

    adt::time_point debug_t2 = adt::now();

    /* debug timing */
    if (debug_timer.has_value()) {
        debug_timer->first += debug_t1 - debug_t0;
        debug_timer->second += debug_t2 - debug_t1;
    }
}

void depth_map_project_sphere(
    std::vector<DepthMapEntry>& depth_map,
    healpix::Healpix_Base2& healpix,
    glm::vec3 center,
    size_t sphere_index,
    glm::vec4 sphere,
    bool is_molecule
) {
    glm::vec3 projection_dir = glm::vec3(sphere) - center;
    float projection_radius = glm::asin(sphere.w / glm::length(projection_dir));

    healpix::pointing dir_pointing;
    dir_pointing.from_vec3(healpix::vec3(projection_dir.x, projection_dir.y, projection_dir.z));

    healpix::rangeset<healpix::int64> projection_pixels_rs = healpix.query_disc(dir_pointing, projection_radius);
    const std::vector<healpix::int64>& projection_pixels = projection_pixels_rs.data();

    /* iter pixel ranges */
    for (size_t pixel_range_index = 0; pixel_range_index < projection_pixels.size(); pixel_range_index += 2) {
        size_t pixels_start = static_cast<size_t>(projection_pixels[pixel_range_index]);
        size_t pixels_end = static_cast<size_t>(projection_pixels[pixel_range_index + 1]);

        /* iter pixels */
        for (size_t pixel = pixels_start; pixel < pixels_end; pixel++) {
            healpix::vec3 pixel_dir_hp = healpix.pix2vec(pixel);
            glm::vec3 pixel_dir = glm::vec3(pixel_dir_hp.x, pixel_dir_hp.y, pixel_dir_hp.z);

            float a = glm::dot(projection_dir, pixel_dir) / glm::dot(pixel_dir, pixel_dir);
            glm::vec3 q = projection_dir - a * pixel_dir;
            float dist = a * glm::length(pixel_dir) - glm::sqrt(sphere.w * sphere.w - glm::length2(q));

            DepthMapEntryValue depth_map_value = DepthMapEntryValue::create(sphere_index, dist, is_molecule);
            depth_map[pixel].update_value(depth_map_value);
        }
    }
}

std::optional<std::pair<size_t, float>> depth_map_entry_molecule_only(DepthMapEntry entry)
{
    if (!entry.is_sphere()) {
        return std::nullopt;
    }

    DepthMapEntryValue value = entry.get_value();
    if (!value.is_molecule) {
        return std::nullopt;
    }

    return std::optional<std::pair<size_t, float>>({ value.index, value.distance });
}

void around_m_spheres_update(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    size_t m_sphere_index,
    size_t t_sphere_index,
    float distance
) {
    std::pair<size_t, float> new_around_m_sphere{ t_sphere_index, distance };

    auto [ around_m_sphere_iter, inserted ] = around_m_spheres.insert({ m_sphere_index, new_around_m_sphere });
    if (!inserted) {
        float around_m_sphere_dist = around_m_sphere_iter->second.second;
        if (distance < around_m_sphere_dist) {
            around_m_sphere_iter->second = new_around_m_sphere;
        }
    }
}

void add_m_spheres_filter_type_none(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    size_t t_sphere_index
) {
    for (DepthMapEntry entry : depth_map) {
        std::optional<std::pair<size_t, float>> molecule_entry = depth_map_entry_molecule_only(entry);
        if (!molecule_entry.has_value()) {
            continue;
        }
        auto [m_sphere_index, dist] = *molecule_entry;

        around_m_spheres_update(around_m_spheres, m_sphere_index, t_sphere_index, dist);
    }
}

void add_m_spheres_filter_min_per_pixel(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    healpix::Healpix_Base2& healpix,
    size_t t_sphere_index,
    float filter_angle,
    float filter_distance
) {
    /* iter depth map */
    for (size_t pixel = 0; pixel < depth_map.size(); pixel++) {
        std::optional<std::pair<size_t, float>> molecule_entry = depth_map_entry_molecule_only(depth_map[pixel]);
        if (!molecule_entry.has_value()) {
            continue;
        }
        auto [m_sphere_index, dist] = *molecule_entry;

        float dist_min = std::numeric_limits<float>::max();

        healpix::vec3 pixel_dir_hp = healpix.pix2vec(pixel);
        healpix::pointing pixel_dir_pointing;
        pixel_dir_pointing.from_vec3(healpix::vec3(pixel_dir_hp.x, pixel_dir_hp.y, pixel_dir_hp.z));

        healpix::rangeset<healpix::int64> other_pixels_rs = healpix.query_disc(pixel_dir_pointing, filter_angle);
        const std::vector<healpix::int64>& other_pixels = other_pixels_rs.data();

        /* iter other pixels ranges */
        for (size_t other_pixel_range_index = 0; other_pixel_range_index < other_pixels.size(); other_pixel_range_index += 2) {
            size_t other_pixels_start = static_cast<size_t>(other_pixels[other_pixel_range_index]);
            size_t other_pixels_end = static_cast<size_t>(other_pixels[other_pixel_range_index + 1]);

            /* iter other pixels */
            for (size_t other_pixel = other_pixels_start; other_pixel < other_pixels_end; other_pixel++) {
                std::optional<std::pair<size_t, float>> other_molecule_entry = depth_map_entry_molecule_only(depth_map[other_pixel]);
                if (!other_molecule_entry.has_value()) {
                    continue;
                }

                float other_dist = other_molecule_entry->second;
                if (other_dist < dist_min) {
                    dist_min = other_dist;
                }
            }
        }

        float treshold = dist_min + filter_distance;
        if (dist <= treshold) {
            around_m_spheres_update(around_m_spheres, m_sphere_index, t_sphere_index, dist);
        }
    }
}

void add_m_spheres_filter_min_per_m_sphere(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4> m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    size_t t_sphere_index,
    glm::vec3 center,
    float filter_angle,
    float filter_distance
) {
    std::unordered_set<size_t> around_indexes;

    for (DepthMapEntry entry : depth_map) {
        std::optional<std::pair<size_t, float>> molecule_entry = depth_map_entry_molecule_only(entry);
        if (!molecule_entry.has_value()) {
            continue;
        }
        size_t m_sphere_index = molecule_entry->first;

        around_indexes.insert(m_sphere_index);
    }

    auto rng = around_indexes | std::views::transform([m_spheres, center] (size_t m_sphere_index) {
        glm::vec4 m_sphere = m_spheres[m_sphere_index];

        glm::vec3 dir = glm::vec3(m_sphere) - center;
        float projection_radius = glm::asin(m_sphere.w / glm::length(dir));

        return std::make_tuple(m_sphere_index, glm::normalize(dir), glm::length(dir), projection_radius);
    });

    std::vector<std::tuple<size_t, glm::vec3, float, float>> around_directions(rng.begin(), rng.end()); /* molecule sphere index, direction, distance, projection radius */

    for (auto [m_sphere_index, dir, dist, radius] : around_directions) {
        float dist_min = std::numeric_limits<float>::max();

        for (auto [other_m_sphere_index, other_dir, other_dist, other_radius] : around_directions) {
            if (glm::dot(dir, other_dir) > glm::cos(filter_angle + (radius + other_radius) / 2)) {
                if (other_dist < dist_min) {
                    dist_min = other_dist;
                }
            }
        }

        float treshold = dist_min + filter_distance;
        if (dist <= treshold) {
            around_m_spheres_update(around_m_spheres, m_sphere_index, t_sphere_index, dist);
        }
    }
}

inline void add_m_spheres_gaussian_min_per_pixel(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    healpix::Healpix_Base2& healpix,
    size_t t_sphere_index,
    float filter_angle
) {
    for (size_t pixel = 0; pixel < depth_map.size(); pixel++) {
        std::optional<std::pair<size_t, float>> molecule_entry = depth_map_entry_molecule_only(depth_map[pixel]);
        if (!molecule_entry.has_value()) {
            continue;
        }
        auto [m_sphere_index, dist] = *molecule_entry;

        float dist_sum = 0;
        float dist_sum2 = 0;
        size_t dist_count = 0;

        healpix::vec3 pixel_dir_hp = healpix.pix2vec(pixel);
        healpix::pointing pixel_dir_pointing;
        pixel_dir_pointing.from_vec3(healpix::vec3(pixel_dir_hp.x, pixel_dir_hp.y, pixel_dir_hp.z));

        healpix::rangeset<healpix::int64> other_pixels_rs = healpix.query_disc(pixel_dir_pointing, filter_angle);
        const std::vector<healpix::int64>& other_pixels = other_pixels_rs.data();

        /* iter other pixels ranges */
        for (size_t other_pixel_range_index = 0; other_pixel_range_index < other_pixels.size(); other_pixel_range_index += 2) {
            size_t other_pixels_start = static_cast<size_t>(other_pixels[other_pixel_range_index]);
            size_t other_pixels_end = static_cast<size_t>(other_pixels[other_pixel_range_index + 1]);

            /* iter other pixels */
            for (size_t other_pixel = other_pixels_start; other_pixel < other_pixels_end; other_pixel++) {
                std::optional<std::pair<size_t, float>> other_molecule_entry = depth_map_entry_molecule_only(depth_map[other_pixel]);
                if (!other_molecule_entry.has_value()) {
                    continue;
                }

                float other_dist = other_molecule_entry->second;
                
                dist_sum += other_dist;
                dist_sum2 += other_dist * other_dist;
                dist_count++;
            }
        }

        float dist_count_f = static_cast<float>(dist_count);
        float dist_avg = dist_sum / dist_count_f;
        float dist_sd = glm::sqrt(dist_sum2 / dist_count_f - dist_avg * dist_avg);

        float treshold = dist_avg + 0.5f * dist_sd;
        if (dist <= treshold) {
            around_m_spheres_update(around_m_spheres, m_sphere_index, t_sphere_index, dist);
        }
    }
}

inline void add_m_spheres_gaussian_min_per_m_sphere(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres,
    std::span<glm::vec4> m_spheres,
    std::vector<DepthMapEntry>& depth_map,
    size_t t_sphere_index,
    glm::vec3 center,
    float filter_angle
) {
    std::unordered_map<size_t, size_t> around_coverage;

    for (DepthMapEntry entry : depth_map) {
        std::optional<std::pair<size_t, float>> molecule_entry = depth_map_entry_molecule_only(entry);
        if (!molecule_entry.has_value()) {
            continue;
        }
        size_t m_sphere_index = molecule_entry->first;

        auto [ m_sphere_coverage, inserted ] = around_coverage.insert({ m_sphere_index, 1 });
        if (!inserted) {
            m_sphere_coverage->second++;
        }
    }

    auto rng = around_coverage | std::views::transform([m_spheres, center] (std::pair<size_t, size_t> m_sphere_iter) {
        auto [ m_sphere_index, m_sphere_coverage ] = m_sphere_iter;

        glm::vec4 m_sphere = m_spheres[m_sphere_index];

        glm::vec3 dir = glm::vec3(m_sphere) - center;
        float projection_radius = glm::asin(m_sphere.w / glm::length(dir));

        return std::make_tuple(m_sphere_index, m_sphere_coverage, glm::normalize(dir), glm::length(dir), projection_radius);
    });

    std::vector<std::tuple<size_t, size_t, glm::vec3, float, float>> around_directions(rng.begin(), rng.end()); /* molecule sphere index, coverage, direction, distance, projection radius */

    for (auto [m_sphere_index, coverage, dir, dist, radius] : around_directions) {
        float dist_sum = 0;
        float dist_sum2 = 0;
        size_t dist_count = 0;

        for (auto [other_m_sphere_index, other_coverage, other_dir, other_dist, other_radius] : around_directions) {
            if (glm::dot(dir, other_dir) > glm::cos(filter_angle + (radius + other_radius) / 2)) {
                dist_sum += other_dist * other_coverage;
                dist_sum2 += other_dist * other_dist * other_coverage;
                dist_count += other_coverage;
            }
        }

        float dist_count_f = static_cast<float>(dist_count);
        float dist_avg = dist_sum / dist_count_f;
        float dist_sd = glm::sqrt(dist_sum2 / dist_count_f - dist_avg * dist_avg);

        float treshold = dist_avg + 0.5f * dist_sd;
        if (dist <= treshold) {
            around_m_spheres_update(around_m_spheres, m_sphere_index, t_sphere_index, dist);
        }
    }
}

std::unordered_set<size_t> compute_around_m_spheres_indexes(
    std::unordered_map<size_t, std::pair<size_t, float>>& around_m_spheres
) {
    auto rng = around_m_spheres | std::views::transform([] (auto around_m_sphere_iter) { return around_m_sphere_iter.first; });
    return std::unordered_set<size_t>(rng.begin(), rng.end());
}