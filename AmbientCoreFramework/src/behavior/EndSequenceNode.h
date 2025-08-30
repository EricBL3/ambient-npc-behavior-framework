/**
 * @file EndSequenceNode.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include "SequenceNode.h"

namespace AmbientCharacterBehavior {

/**
 * @brief A node that marks the end of a sequence.
 */
class EndSequenceNode : public SequenceNode {
public:
    explicit EndSequenceNode(int32_t node_id) : SequenceNode(node_id) {}

    /**
     *
     * @return SequenceNodeType::END_SEQUENCE_NODE
     */
    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::END_SEQUENCE_NODE;
    }
};

}