#pragma once


#include "algorithm/debug_utils.hpp"

#include "bitarray/lazy_bitarray_3d.hpp"
#include "bitarray/boundary_points.hpp"

#include "union_find/union_find.hpp"


#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtx/std_based_type.hpp"
#include "glm/common.hpp"


#include <vector>
#include <unordered_set>
#include <utility>
#include <ranges>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <iostream>



/*
 * returns : (voxels, start of voxel grid)
 */
template<size_t E1, size_t E2, size_t E3>
std::pair<lazy_bitarray_3d, glm::vec3> compute_voxels(
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres,
    std::span<glm::vec4, E3> roi_spheres,
    float voxel_size,
    float probe,
    bool m_spheres_only_intersecting,
    bool segmentation,
    bool force_tunnel,
    bool debug_info = false
);

inline void voxels_render_sphere(
    lazy_bitarray_3d& voxels,
    glm::vec3 voxels_start,
    float voxel_size,
    glm::size3_t voxels_count,
    glm::vec3 center,
    float radius,
    bool value
);

inline void voxels_render_sphere_no_transform(
    lazy_bitarray_3d& voxels,
    glm::size3_t voxels_count,
    glm::vec3 center,
    float radius,
    bool value
);

inline std::pair<size_t, size_t> voxels_range(
    size_t voxel_count,
    float center,
    float radius
);

template<size_t E1>
void compute_segmentation(
    lazy_bitarray_3d& voxels,
    std::span<glm::vec4, E1> t_spheres,
    glm::vec3 voxels_start,
    float voxel_size
);



template<size_t E1, size_t E2, size_t E3>
std::pair<lazy_bitarray_3d, glm::vec3> compute_voxels(
    std::span<glm::vec4, E1> t_spheres,
    std::span<glm::vec4, E2> m_spheres,
    std::span<glm::vec4, E3> roi_spheres,
    float voxel_size,
    float probe,
    bool m_spheres_only_intersecting,
    bool segmentation,
    [[maybe_unused]] bool force_tunnel,
    bool debug_info
) {
    glm::vec3 bbox_min(std::numeric_limits<float>::max());
    glm::vec3 bbox_max(std::numeric_limits<float>::lowest());

    auto bbox_sphere = [&bbox_min, &bbox_max] (glm::vec4 sphere) {
        glm::vec3 center(sphere);
        float radius = sphere.w;

        bbox_min = glm::min(bbox_min, center - radius);
        bbox_max = glm::max(bbox_max, center + radius);
    };

    std::ranges::for_each(t_spheres, bbox_sphere);
    std::ranges::for_each(roi_spheres, bbox_sphere);

    bbox_min -= probe;
    bbox_max += probe;

    glm::vec3 bbox_center = (bbox_min + bbox_max) / 2.0f;
    glm::vec3 bbox_size = bbox_max - bbox_min;

    glm::size3_t voxels_count = glm::ceil(bbox_size / voxel_size);
    glm::vec3 voxels_start = bbox_center - glm::vec3(voxels_count) * voxel_size / 2.0f;

    if (debug_info) {
        std::cout << "[debug] voxel count : (" << voxels_count.x << ", " << voxels_count.y << ", " << voxels_count.z << ")\n";
    }

    // std::vector<glm::size3_t> voxels_layers_sizing{ voxels_count, glm::size3_t(64), glm::size3_t(16) }; // [todo] sizing
    // std::vector<glm::size3_t> voxels_layers_sizing{ voxels_count, glm::size3_t(16) }; // [todo] sizing
    std::vector<glm::size3_t> voxels_layers_sizing{ voxels_count }; // [todo] sizing

    lazy_bitarray_3d voxels(std::move(voxels_layers_sizing), false);

    adt::time_point debug_t0 = adt::now();

    std::ranges::for_each(t_spheres, [&voxels, voxels_start, voxel_size, voxels_count] (glm::vec4 sphere) {
        voxels_render_sphere(voxels, voxels_start, voxel_size, voxels_count, glm::vec3(sphere), sphere.w, true);
    });

    adt::time_point debug_t1 = adt::now();
    if (debug_info) {
        std::cout << "[debug] voxels tunnel spheres\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t1 - debug_t0);
        std::cout << "\n";
    }

    std::ranges::for_each(roi_spheres, [&voxels, voxels_start, voxel_size, voxels_count] (glm::vec4 sphere) {
        voxels_render_sphere(voxels, voxels_start, voxel_size, voxels_count, glm::vec3(sphere), sphere.w, true);
    });

    adt::time_point debug_t2 = adt::now();
    if (debug_info) {
        std::cout << "[debug] voxels roi spheres\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t2 - debug_t1);
        std::cout << "\n";
    }

    for (glm::vec4 m_sphere : m_spheres) {
        glm::vec3 center(m_sphere);
        float radius = m_sphere.w + probe;

        // [note] filters out spheres outside of bounding box
        glm::vec3 bbox_closest = glm::clamp(center, bbox_min, bbox_max);
        if (glm::distance(bbox_closest, center) > radius) {
            continue;
        }

        if (m_spheres_only_intersecting) {
            // [note] filters out spheres not intersecting with roi or tunnel
            bool skip = true;

            for (glm::vec4 roi_sphere : roi_spheres) {
                if (glm::distance(glm::vec3(roi_sphere), center) <= radius + roi_sphere.w) {
                    skip = false;
                    break;
                }
            }

            if (skip) {
                for (glm::vec4 t_sphere : t_spheres) {
                    if (glm::distance(glm::vec3(t_sphere), center) <= radius + t_sphere.w) {
                        skip = false;
                        break;
                    }
                }

                if (skip) {
                    continue;
                }
            }
        }

        voxels_render_sphere(voxels, voxels_start, voxel_size, voxels_count, center, radius, false);
    };

    adt::time_point debug_t3 = adt::now();
    if (debug_info) {
        std::cout << "[debug] voxels molecule spheres\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t3 - debug_t2);
        std::cout << "\n";
    }

    if (segmentation) {
        compute_segmentation(voxels, t_spheres, voxels_start, voxel_size);
    }

    adt::time_point debug_t4 = adt::now();
    if (debug_info) {
        std::cout << "[debug] voxels segmentation\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t4 - debug_t3);
        std::cout << "\n";
    }

    std::vector<glm::size3_t> boundary_points = lazy_bitarray_3d_boundary_points(voxels);

    adt::time_point debug_t5 = adt::now();
    if (debug_info) {
        std::cout << "[debug] voxels find boundary points\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t5 - debug_t4);
        std::cout << "\n";
    }

    // [note] original boundary voxels finding
    // for (size_t z = 1; z < voxels_count.z; z++) {
    //     for (size_t y = 1; y < voxels_count.y; y++) {
    //         for (size_t x = 1; x < voxels_count.x; x++) {
    //             if (voxels.get_bit(glm::size3_t(x, y, z))) {
    //                 bool is_boundary = 
    //                     !voxels.get_bit(glm::size3_t(x + 1, y, z)) ||
    //                     !voxels.get_bit(glm::size3_t(x - 1, y, z)) ||
    //                     !voxels.get_bit(glm::size3_t(x, y + 1, z)) ||
    //                     !voxels.get_bit(glm::size3_t(x, y - 1, z)) ||
    //                     !voxels.get_bit(glm::size3_t(x, y, z + 1)) ||
    //                     !voxels.get_bit(glm::size3_t(x, y, z - 1));

    //                 if (is_boundary) {
    //                     count++;
    //                 }
    //             }
    //         }
    //     }
    // }

    float probe_radius_transformed = probe / voxel_size;

    std::ranges::for_each(boundary_points, [&voxels, voxels_count, probe_radius_transformed] (glm::size3_t point) {
        voxels_render_sphere_no_transform(voxels, voxels_count, glm::vec3(point) + 0.5f, probe_radius_transformed, true);
    });

    adt::time_point debug_t6 = adt::now();
    if (debug_info) {
        std::cout << "[debug] voxels boundary spheres\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t6 - debug_t5);
        std::cout << "\n";
    }

    if (force_tunnel) {
        std::ranges::for_each(t_spheres, [&voxels, voxels_start, voxel_size, voxels_count] (glm::vec4 sphere) {
            voxels_render_sphere(voxels, voxels_start, voxel_size, voxels_count, glm::vec3(sphere), sphere.w, true);
        });
    }

    adt::time_point debug_t7 = adt::now();
    if (debug_info) {
        std::cout << "[debug] voxels force tunnel spheres\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t7 - debug_t6);
        std::cout << "\n";
    }

    return { std::move(voxels), voxels_start };
}

void voxels_render_sphere(
    lazy_bitarray_3d& voxels,
    glm::vec3 voxels_start,
    float voxel_size,
    glm::size3_t voxels_count,
    glm::vec3 center,
    float radius,
    bool value
) {
    center = (center - voxels_start) / voxel_size;
    radius /= voxel_size;

    voxels_render_sphere_no_transform(voxels, voxels_count, center, radius, value);
}

void voxels_render_sphere_no_transform(
    lazy_bitarray_3d& voxels,
    glm::size3_t voxels_count,
    glm::vec3 center,
    float radius,
    bool value
) {
    auto [min_z, max_z] = voxels_range(voxels_count.z, center.z, radius);

    for (size_t z = min_z; z < max_z; z++) {
        float dz = static_cast<float>(z) + 0.5f - center.z;
        float dy_max = glm::sqrt(radius * radius - dz * dz);

        auto [min_y, max_y] = voxels_range(voxels_count.y, center.y, dy_max);

        for (size_t y = min_y; y < max_y; y++) {
            float dy = static_cast<float>(y) + 0.5f - center.y;
            float dx_max = glm::sqrt(radius * radius - dz * dz - dy * dy);

            auto [min_x, max_x] = voxels_range(voxels_count.x, center.x, dx_max);

            for (size_t x = min_x; x < max_x; x++) {
                voxels.set_bit(glm::size3_t(x, y, z), value);
            }
        }
    }
}

std::pair<size_t, size_t> voxels_range(
    size_t voxel_count,
    float center,
    float radius
) {
    float min = center - radius;
    if (min >= voxel_count - 0.5) {
        return { 0, 0 };
    }
    size_t min_index = (min < 0.5f) ? 0 : static_cast<size_t>(min + 0.5f);

    float max = center + radius;
    if (max < 0.5f) {
        return { 0, 0 };
    }
    size_t max_index = (max > voxel_count - 0.5) ? voxel_count : static_cast<size_t>(max + 0.5f);

    return { min_index, max_index };
}

template<size_t E1>
void compute_segmentation(
    lazy_bitarray_3d& voxels,
    std::span<glm::vec4, E1> t_spheres,
    glm::vec3 voxels_start,
    float voxel_size
) {
    glm::size3_t size = voxels.get_size();

    std::vector<uint16_t> segmentation(size.x * size.y * size.z);

    union_find labeling;

    for (size_t z = 0; z < size.z; z++) {
        for (size_t y = 0; y < size.y; y++) {
            for (size_t x = 0; x < size.x; x++) {
                if (voxels.get_bit(glm::size3_t(x, y, z))) {
                    uint16_t xn = (x == 0) ? 0 : segmentation[x - 1 + y * size.x + z * size.y * size.x];
                    uint16_t yn = (y == 0) ? 0 : segmentation[x + (y - 1) * size.x + z * size.y * size.x];
                    uint16_t zn = (z == 0) ? 0 : segmentation[x + y * size.x + (z - 1) * size.y * size.x];

                    if (xn == 0 && yn == 0 && zn == 0) {
                        segmentation[x + y * size.x + z * size.y * size.x] = labeling.new_index();
                    } else if (xn == 0 && yn == 0 && zn != 0) {
                        segmentation[x + y * size.x + z * size.y * size.x] = zn;
                    } else if (xn == 0 && yn != 0 && zn == 0) {
                        segmentation[x + y * size.x + z * size.y * size.x] = yn;
                    } else if (xn != 0 && yn == 0 && zn == 0) {
                        segmentation[x + y * size.x + z * size.y * size.x] = xn;
                    } else if (xn != 0 && yn != 0 && zn == 0) {
                        segmentation[x + y * size.x + z * size.y * size.x] = labeling.merge(xn, yn);
                    } else if (xn != 0 && yn == 0 && zn != 0) {
                        segmentation[x + y * size.x + z * size.y * size.x] = labeling.merge(xn, zn);
                    } else if (xn == 0 && yn != 0 && zn != 0) {
                        segmentation[x + y * size.x + z * size.y * size.x] = labeling.merge(yn, zn);
                    } else {
                        segmentation[x + y * size.x + z * size.y * size.x] = labeling.merge(xn, labeling.merge(yn, zn));
                    }
                }
            }
        }
    }

    size_t label_count = labeling.size();

    std::vector<uint16_t> labeling_map(label_count);

    for (size_t label = 0; label < label_count; label++) {
        labeling_map[label] = labeling.find(static_cast<uint16_t>(label));
    }

    std::unordered_set<uint16_t> preserve_labels;

    for (glm::vec4 t_sphere : t_spheres) {
        glm::size3_t index((glm::vec3(t_sphere) - voxels_start) / voxel_size);
        preserve_labels.insert(labeling_map[segmentation[index.x + index.y * size.x + index.z * size.y * size.x]]);
    }

    for (size_t z = 0; z < size.z; z++) {
        for (size_t y = 0; y < size.y; y++) {
            for (size_t x = 0; x < size.x; x++) {
                uint16_t label_orig = segmentation[x + y * size.x + z * size.y * size.x];
                if (label_orig != 0) {
                    uint16_t label = labeling_map[label_orig];

                    if (!preserve_labels.contains(label)) {
                        voxels.set_bit(glm::size3_t(x, y, z), false);
                    }
                }
            }
        }
    }
}