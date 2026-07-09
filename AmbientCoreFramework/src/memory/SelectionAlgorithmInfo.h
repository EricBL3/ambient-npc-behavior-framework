#pragma once
#include <string>

namespace AmbientCharacterBehavior {

    struct SelectionAlgorithmInfo {
        int32_t npc_id{};
        std::string npc_name{};
        int32_t sequence_id = -1;
        int32_t current_node_id = -1;
        int32_t action_id = -1;
    };

}
