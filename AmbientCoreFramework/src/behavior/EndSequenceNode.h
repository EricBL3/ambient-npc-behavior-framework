/**
 * @file EndSequenceNode.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include "SequenceNode.h"

/**
 * @brief A node that marks the end of a sequence.
 */
class EndSequenceNode : public SequenceNode {
public:
    EndSequenceNode(const int node_id) : SequenceNode(node_id) {}

    /**
     *
     * @return SequenceNodeType::END_SEQUENCE_NODE
     */
    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::END_SEQUENCE_NODE;
    }
};