#include "marching_cubes/marching_cubes.hpp"


#include "marching_cubes/marching_cubes_data.hpp"

#include "algorithm/debug_utils.hpp"


#include <iostream>
#include <unordered_set>
#include <ranges>



marching_cubes::marching_cubes(glm::vec3 offset_, float cube_size_, glm::size3_t cubes_count_)
:
    offset(offset_),
    cube_size(cube_size_),
    cubes_count(cubes_count_)
{}

void marching_cubes::add_cube(glm::size3_t index, uint8_t config)
{
    if (config == 0 || config == 0xff) {
        return;
    }

    glm::vec3 pos(index);

    const std::array<uint8_t, 15>& triangles = marching_cubes_data::cube_configs[config];

    for (size_t triangle_index = 0; triangle_index < 15; triangle_index += 3) {
        if (triangles[triangle_index] == 20) {
            break;
        }

        uint8_t edge0_index = triangles[triangle_index];
        uint8_t edge1_index = triangles[triangle_index + 1];
        uint8_t edge2_index = triangles[triangle_index + 2];

        size_t vertex0_index = edge_get_vertex_index(index, edge0_index);
        size_t vertex1_index = edge_get_vertex_index(index, edge1_index);
        size_t vertex2_index = edge_get_vertex_index(index, edge2_index);

        glm::vec3 vertex0;

        auto [ vertex0_iter, vertex0_inserted ] = vertices.insert({ vertex0_index, { glm::vec3(0), glm::vec3(0) } });
        std::pair<glm::vec3, glm::vec3>& vertex0_data = vertex0_iter->second;

        if (vertex0_inserted) {
            vertex0 = marching_cubes_data::edge_mid_vertex[edge0_index];
            vertex0 = (vertex0 + pos) * cube_size + offset;

            vertex0_data.first = vertex0;
        } else {
            vertex0 = vertex0_data.first;
        }

        glm::vec3 vertex1;

        auto [ vertex1_iter, vertex1_inserted ] = vertices.insert({ vertex1_index, { glm::vec3(0), glm::vec3(0) } });
        std::pair<glm::vec3, glm::vec3>& vertex1_data = vertex1_iter->second;

        if (vertex1_inserted) {
            vertex1 = marching_cubes_data::edge_mid_vertex[edge1_index];
            vertex1 = (vertex1 + pos) * cube_size + offset;

            vertex1_data.first = vertex1;
        } else {
            vertex1 = vertex1_data.first;
        }

        glm::vec3 vertex2;

        auto [ vertex2_iter, vertex2_inserted ] = vertices.insert({ vertex2_index, { glm::vec3(0), glm::vec3(0) } });
        std::pair<glm::vec3, glm::vec3>& vertex2_data = vertex2_iter->second;

        if (vertex2_inserted) {
            vertex2 = marching_cubes_data::edge_mid_vertex[edge2_index];
            vertex2 = (vertex2 + pos) * cube_size + offset;

            vertex2_data.first = vertex2;
        } else {
            vertex2 = vertex2_data.first;
        }

        glm::vec3 normal = glm::normalize(glm::cross(vertex2 - vertex0, vertex1 - vertex0));

        vertex0_data.second += normal;
        vertex1_data.second += normal;
        vertex2_data.second += normal;

        faces.push_back(vertex0_index);
        faces.push_back(vertex1_index);
        faces.push_back(vertex2_index);
    }
}

std::vector<glm::vec3> marching_cubes::get_triangles(size_t smoothing_iter_count, bool debug_info) const
{
    adt::time_point debug_t0 = adt::now();

    /* reindex vertices */
    std::vector<std::pair<glm::vec3, glm::vec3>> vertices_unindexed;
    std::unordered_map<size_t, size_t> vertex_reindexing;

    size_t next_vertex_index = 0;

    for (auto [ vertex_index, vertex ] : vertices) {
        vertex_reindexing.emplace(vertex_index, next_vertex_index);
        vertices_unindexed.push_back(vertex);

        next_vertex_index++;
    }

    /* apply vertex reindexing on faces */
    auto rng = faces | std::views::transform([&vertex_reindexing] (size_t index) {
        return vertex_reindexing.at(index);
    });

    std::vector<size_t> faces_unindexed(rng.begin(), rng.end());

    /* compute normals */
    for (auto& vertex : vertices_unindexed) {
        vertex.second = glm::normalize(vertex.second);
    }

    /* compute vertex neighbours */
    std::vector<std::unordered_set<size_t>> vertex_neighbours(vertices_unindexed.size());

    for (size_t face_index = 0; face_index < faces_unindexed.size(); face_index += 3) {
        size_t vertex0_index = faces_unindexed[face_index];
        size_t vertex1_index = faces_unindexed[face_index + 1];
        size_t vertex2_index = faces_unindexed[face_index + 2];

        vertex_neighbours.at(vertex0_index).insert({ vertex1_index, vertex2_index });
        vertex_neighbours.at(vertex1_index).insert({ vertex0_index, vertex2_index });
        vertex_neighbours.at(vertex2_index).insert({ vertex0_index, vertex1_index });
    }

    adt::time_point debug_t1 = adt::now();
    if (debug_info) {
        std::cout << "[debug] surface compute mesh\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t1 - debug_t0);
        std::cout << "\n";
    }

    /* smoothing */
    for (size_t i = 0; i < smoothing_iter_count; i++) {
        std::vector<std::pair<glm::vec3, glm::vec3>> vertices_unindexed_smooth;

        for (size_t vertex_index = 0; vertex_index < vertices_unindexed.size(); vertex_index++) {
            auto [ pos_smooth, normal_smooth ] = vertices_unindexed[vertex_index];
            pos_smooth *= 2;
            normal_smooth *= 2;

            const std::unordered_set<size_t>& neighbours = vertex_neighbours.at(vertex_index);

            for (size_t neighbour_vertex_index : neighbours) {
                auto [ neighbour_pos, neighbour_normal ] = vertices_unindexed[neighbour_vertex_index];
                pos_smooth += neighbour_pos;
                normal_smooth += neighbour_normal;
            }

            vertices_unindexed_smooth.emplace_back(pos_smooth / (2.0f + static_cast<float>(neighbours.size())), glm::normalize(normal_smooth));
        }

        vertices_unindexed = std::move(vertices_unindexed_smooth);
    }

    adt::time_point debug_t2 = adt::now();
    if (debug_info) {
        std::cout << "[debug] surface smoothing\n";
        std::cout << "[debug] ";
        adt::duration_print_as_ns(debug_t2 - debug_t1);
        std::cout << "\n";
    }

    /* compute triangles */
    std::vector<glm::vec3> triangles;

    for (size_t vertex_index : faces_unindexed) {
        auto [ pos, normal ] = vertices_unindexed[vertex_index];
        triangles.push_back(pos);
        triangles.push_back(normal);
    }

    return triangles;
}

size_t marching_cubes::edge_get_vertex_index(glm::size3_t index, uint8_t edge_index) const
{
    index += marching_cubes_data::edge_index_add[edge_index];

    size_t index_1d = ((index.z * (cubes_count.y + 1) * (cubes_count.x + 1) + (cubes_count.x + 1) * index.y + index.x) << 2);

    if (edge_index < 8) {
        if (edge_index % 2 == 0) {
            return index_1d | static_cast<size_t>(0b00);
        } else {
            return index_1d | static_cast<size_t>(0b01);
        }
    } else {
        return index_1d | static_cast<size_t>(0b10);
    }
}