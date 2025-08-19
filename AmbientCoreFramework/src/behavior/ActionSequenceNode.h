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
     * @brief A reference to the action that will be executed
     */
    Action* target_action;

public:
    /**
     *
     * @param node_id The identifier of the action sequence node that will be created
     * @param action A reference to the target action
     */
    ActionSequenceNode(int node_id, Action* action) : SequenceNode(node_id), target_action(action) {}

    Action* GetTargetAction() const { return target_action; }

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