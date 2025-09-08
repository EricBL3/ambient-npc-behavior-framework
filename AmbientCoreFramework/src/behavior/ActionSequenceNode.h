#pragma once
#include "SequenceNode.h"
#include "Action.h"

namespace AmbientCharacterBehavior {

/**
 * @brief A node that stores a reference to an action that will be executed.
 */
class ActionSequenceNode : public SequenceNode {
private:

    int32_t target_action_id;

public:
    /**
     * @throws std::invalid_argument if node_id < 0
     */
    ActionSequenceNode(int32_t node_id, int32_t action_id) : SequenceNode(node_id), target_action_id(action_id) {}

    //TODO: Missing implementation which requires a content manager class.
    [[nodiscard]]
    Action& GetTargetAction() const;

    int32_t GetTargetActionId() const { return target_action_id; }

    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::ACTION_NODE;
    }
};

}