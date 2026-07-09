#pragma once
#include <cstdint>
#include <optional>

namespace AmbientCharacterBehavior {

    struct RecencyCandidate
    {
        int32_t id;
        std::optional<int64_t> last_used;
    };

}
