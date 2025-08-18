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
    int node_id;

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
    explicit SequenceNode(const int node_id)
    {
        if(node_id < 0)
        {
            throw std::invalid_argument("SequenceNode: node id cannot be negative, got " + std::to_string(node_id));
        }

        this->node_id = node_id;
        has_completed = false;
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

    int GetNodeId() const
    {
        return node_id;
    }

    bool GetHasCompleted() const
    {
        return has_completed;
    }
};
