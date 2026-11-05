#include "algorithm/settings.hpp"



bool AlgorithmSettings::operator==(const AlgorithmSettings& other) const
{
    return probe == other.probe
        && healpix_nside == other.healpix_nside
        && depth_map_use_probe == other.depth_map_use_probe
        && depth_map_filter == other.depth_map_filter
        && voxel_size == other.voxel_size
        && molecule_spheres_only_intersecting == other.molecule_spheres_only_intersecting
        && force_tunnel == other.force_tunnel
        && smoothing_iter_count == other.smoothing_iter_count;
}