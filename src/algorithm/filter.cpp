#include "algorithm/filter.hpp"



#include <cstdint>



DepthMapFilterMin DepthMapFilterMin::create(float angle, float distance, bool per_pixel)
{
    return DepthMapFilterMin {
        .angle = angle,
        .distance = distance,
        .per_pixel = per_pixel
    };
}

DepthMapFilterGaussian DepthMapFilterGaussian::create(float angle, bool per_pixel)
{
    return DepthMapFilterGaussian {
        .angle = angle,
        .per_pixel = per_pixel
    };
}


DepthMapFilter::DepthMapFilter() : ft(DepthMapFilterNone {}) {}
DepthMapFilter::DepthMapFilter(DepthMapFilterNone ft_) : ft(ft_) {}
DepthMapFilter::DepthMapFilter(DepthMapFilterMin ft_) : ft(ft_) {}
DepthMapFilter::DepthMapFilter(DepthMapFilterGaussian ft_) : ft(ft_) {}

DepthMapFilterType DepthMapFilter::type() const { return static_cast<DepthMapFilterType>(ft.index()); }

DepthMapFilterNone DepthMapFilter::get_none() const { return std::get<0>(ft); }
DepthMapFilterMin DepthMapFilter::get_min() const { return std::get<1>(ft); }
DepthMapFilterGaussian DepthMapFilter::get_gaussian() const { return std::get<2>(ft); }

bool DepthMapFilter::operator==(const DepthMapFilter& other) const
{
    size_t index = ft.index();

    if (index != other.ft.index()) {
        return false;
    }

    if (index == 0) {
        return true;
    } else if (index == 1) {
        const DepthMapFilterMin filter_min = std::get<1>(ft);
        const DepthMapFilterMin other_filter_min = std::get<1>(other.ft);

        return filter_min.angle == other_filter_min.angle && filter_min.distance == other_filter_min.distance && filter_min.per_pixel == other_filter_min.per_pixel;
    } else {
        const DepthMapFilterGaussian filter_gaussian = std::get<2>(ft);
        const DepthMapFilterGaussian other_filter_gaussian = std::get<2>(other.ft);

        return filter_gaussian.angle == other_filter_gaussian.angle && filter_gaussian.per_pixel == other_filter_gaussian.per_pixel;
    }
}