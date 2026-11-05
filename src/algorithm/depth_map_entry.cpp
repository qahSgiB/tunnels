#include "algorithm/depth_map_entry.hpp"


#include <utility>



DepthMapEntryValue DepthMapEntryValue::create(size_t index, float distance, bool is_molecule)
{
    return DepthMapEntryValue {
        .index = index,
        .distance = distance,
        .is_molecule = is_molecule
    };
}


DepthMapEntry::DepthMapEntry() : state() {}

bool DepthMapEntry::is_sphere() const { return state.has_value(); }

DepthMapEntryValue& DepthMapEntry::get_value() { return *state; }
const DepthMapEntryValue& DepthMapEntry::get_value() const { return *state; }

void DepthMapEntry::update_value(DepthMapEntryValue value)
{
    if (!state.has_value() || value.distance < state->distance) {
        state = value;
    }
}