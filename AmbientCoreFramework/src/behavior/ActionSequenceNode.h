/**
 * @file ActionSequenceNode.h
 * @brief
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include "SequenceNode.h"
#include "Action.h"

namespace AmbientCharacterBehavior {

/**
 * @brief A node that stores a reference to an action that will be executed.
 */
class ActionSequenceNode : public SequenceNode {
private:
    /**
     * @brief The identifier of the action that will be executed by this node.
     *
     */
    int32_t target_action_id;

public:
    /**
     *
     * @param node_id The identifier of the action sequence node that will be created
     * @param action_id The identifier of the target action
     */
    ActionSequenceNode(int32_t node_id, int32_t action_id) : SequenceNode(node_id), target_action_id(action_id) {}

    //TODO: Missing implementation which requires a content manager class.
    [[nodiscard]]
    Action& GetTargetAction() const;

    int32_t GetTargetActionId() const { return target_action_id; }

    /**
     *
     * @return SequenceNodeType::ACTION_NODE
     */
    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::ACTION_NODE;
    }
};

}