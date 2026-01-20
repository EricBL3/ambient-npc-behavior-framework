#include "FrameworkEntity.h"
#include <algorithm>
#include <stdexcept>
using namespace AmbientCharacterBehavior;

FrameworkEntity::FrameworkEntity(void* entity_handle, int32_t entity_id, std::string name) :
    entity_handle(entity_handle), entity_id(entity_id), name(std::move(name))
{
    // validate entity_id invariant
    if (entity_id < 0)
    {
        throw std::invalid_argument("FrameworkEntity: entity_id cannot be negative");
    }

    // validate entity_handle invariant
    if (entity_handle == nullptr)
    {
        throw std::invalid_argument("FrameworkEntity: handle cannot be null");
    }
}

int32_t FrameworkEntity::GetStateValue(int32_t state_key)
{
    const auto state_iterator = state.find(state_key);
    if (state_iterator == state.end())
    {
        throw std::runtime_error("FrameworkEntity[" + std::to_string(entity_id) + "]: State_key: " +
            std::to_string(state_key) + " not found");
    }

    return state_iterator->second;
}

void FrameworkEntity::SetStateValue(int32_t state_key, int32_t value)
{
    state.insert_or_assign(state_key, value);
}
