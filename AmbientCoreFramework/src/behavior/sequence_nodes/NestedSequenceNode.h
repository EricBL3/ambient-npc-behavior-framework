#pragma once
#include "SequenceNode.h"
#include "../Sequence.h"

namespace AmbientCharacterBehavior {
/**
 * @brief A node that stores the reference to another sequence to be executed as a subsequence.
 */
class NestedSequenceNode : public SequenceNode {
private:

    int32_t target_sequence_id;

public:
    /**
     * @throws std::invalid_argument if node_id < 0
     */
    NestedSequenceNode(const int32_t node_id, int32_t sequence_id) :
        SequenceNode(node_id), target_sequence_id(sequence_id) {}

    //TODO: Missing implementation which requires a content manager class.
    [[nodiscard]]
    Sequence& GetTargetSequence() const;

    int32_t GetTargetSequenceId() const { return target_sequence_id; }

    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::NESTED_SEQUENCE_NODE;
    }
};

}