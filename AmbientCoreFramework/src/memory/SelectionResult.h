#pragma once
#include <cstdint>
#include <optional>
#include <string_view>

namespace AmbientCharacterBehavior {

struct SelectionResult
{
    std::optional<int32_t> selected_id;
    std::string_view branch_fired;
};

}
