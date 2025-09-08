#pragma once
#include <stdexcept>
#include <string>

#include "SequenceNodeType.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Abstract base class for sequence nodes in the behavior framework.
 *
 * @see ActionSequenceNode, NestedSequenceNode, EndSequenceNode
 */
class SequenceNode {
protected:
    /**
     * @invariant node_id >= 0
     */
    int32_t node_id;

    bool has_completed_execution;

public:
    /**
     * @throws std::invalid_argument if node_id < 0
     */
    explicit SequenceNode(int32_t node_id) : node_id(node_id), has_completed_execution(false)
    {
        if(node_id < 0)
        {
            throw std::invalid_argument("SequenceNode: node id cannot be negative, got " + std::to_string(node_id));
        }
    }

    virtual ~SequenceNode() = default;

    virtual SequenceNodeType GetNodeType() const = 0;

    void ResetCompletion()
    {
        has_completed_execution = false;
    }

    void MarkAsCompleted()
    {
        has_completed_execution = true;
    }

    int32_t GetNodeId() const
    {
        return node_id;
    }

    bool HasCompletedExecution() const
    {
        return has_completed_execution;
    }
};

}