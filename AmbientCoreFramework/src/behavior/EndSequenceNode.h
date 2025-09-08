#pragma once
#include "SequenceNode.h"

namespace AmbientCharacterBehavior {

/**
 * @brief A node that marks the end of a sequence.
 */
class EndSequenceNode : public SequenceNode {
public:
    /**
     * @throws std::invalid_argument if node_id < 0
     */
    explicit EndSequenceNode(int32_t node_id) : SequenceNode(node_id) {}

    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::END_SEQUENCE_NODE;
    }
};

}