#pragma once



#include <variant>
#include <cstddef>




struct DepthMapFilterNone {};

struct DepthMapFilterMin
{
    float angle;
    float distance;
    bool per_pixel;

    static DepthMapFilterMin create(float angle, float distance, bool per_pixel);
};

struct DepthMapFilterGaussian
{
    float angle;
    bool per_pixel;

    static DepthMapFilterGaussian create(float angle, bool per_pixel);
};

enum class DepthMapFilterType : size_t
{
    None = 0,
    Min = 1,
    Gaussian = 2,
};

class DepthMapFilter
{
    std::variant<DepthMapFilterNone, DepthMapFilterMin, DepthMapFilterGaussian> ft;

public:
    DepthMapFilter();
    DepthMapFilter(DepthMapFilterNone ft_);
    DepthMapFilter(DepthMapFilterMin ft_);
    DepthMapFilter(DepthMapFilterGaussian ft_);

    DepthMapFilterType type() const;

    DepthMapFilterNone get_none() const;
    DepthMapFilterMin get_min() const;
    DepthMapFilterGaussian get_gaussian() const;

    bool operator==(const DepthMapFilter& other) const;
};
