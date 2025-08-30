/**
 * @file SequenceNode.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

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
     * @brief The identifier of the sequence node.
     * @invariant node_id >= 0
     */
    int32_t node_id;

    /**
     * @brief Determines if the sequence node has completed execution.
     */
    bool has_completed;

public:
    /**
     * @brief Constructs a new sequence node with the specified identifier
     * @param node_id The identifier of the sequence node that will be created
     * @throws std::invalid_argument if node_id < 0
     */
    explicit SequenceNode(int32_t node_id) : node_id(node_id), has_completed(false)
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
        has_completed = false;
    }

    void MarkAsCompleted()
    {
        has_completed = true;
    }

    int32_t GetNodeId() const
    {
        return node_id;
    }

    bool IsCompleted() const
    {
        return has_completed;
    }
};

}