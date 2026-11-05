#pragma once



#include "algorithm/filter.hpp"


#include <cstdint>
#include <cstddef>



// [note] default example settings
// float probe = 0.7f;
// int64_t healpix_nside = 16;
// bool depth_map_use_probe = false;
// DepthMapFilter depth_map_filter(DepthMapFilterMin::create(glm::radians(60.0f), 2.0f, false));
// float voxel_size = 0.2f;
// float force_tunnel = true;
// size_t smoothing_iter_count = 3;

struct AlgorithmSettings
{
    float probe;
    int64_t healpix_nside;
    bool depth_map_use_probe;
    DepthMapFilter depth_map_filter;
    float voxel_size;
    bool molecule_spheres_only_intersecting;
    bool segmentation;
    bool force_tunnel;
    size_t smoothing_iter_count;
    
    bool operator==(const AlgorithmSettings& other) const;
};
