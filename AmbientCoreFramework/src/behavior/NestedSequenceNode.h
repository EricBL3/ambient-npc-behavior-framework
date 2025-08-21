/**
 * @file NestedSequenceNode.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include "SequenceNode.h"
#include "Sequence.h"

namespace AmbientCharacterBehavior {
/**
 * @brief A node that stores the reference to another sequence to be executed as a subsequence.
 */
class NestedSequenceNode : public SequenceNode {
private:
    /**
     * @brief The identifier of the sequence that will be executed as a subsequence.
     */
    int target_sequence_id;

public:
    /**
     *
     * @param node_id The identifier of the nested sequence node that will be created
     * @param sequence_id The identifier of the target sequence.
     */
    NestedSequenceNode(const int node_id, int sequence_id) : SequenceNode(node_id), target_sequence_id(sequence_id) {}

    //TODO: Missing implementation which requires a content manager class.
    [[nodiscard]]
    Sequence& GetTargetSequence() const;

    int GetTargetSequenceId() const { return target_sequence_id; }

    /**
     *
     * @return SequenceNodeType::NESTED_SEQUENCE_NODE
     */
    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::NESTED_SEQUENCE_NODE;
    }
};

}