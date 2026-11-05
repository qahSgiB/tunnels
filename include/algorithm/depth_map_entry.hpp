#pragma once


#include <optional>



struct DepthMapEntryValue
{
    size_t index;
    float distance;
    bool is_molecule;

    static DepthMapEntryValue create(size_t index, float distance, bool is_molecule);
};

class DepthMapEntry
{
    std::optional<DepthMapEntryValue> state;

public:
    DepthMapEntry();

    bool is_sphere() const;

    DepthMapEntryValue& get_value();
    const DepthMapEntryValue& get_value() const;

    void update_value(DepthMapEntryValue value);
};