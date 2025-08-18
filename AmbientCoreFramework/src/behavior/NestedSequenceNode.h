/**
 * @file NestedSequenceNode.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include "SequenceNode.h"

class Sequence;

/**
 * @brief A node that stores the reference to another sequence to be executed as a subsequence.
 */
class NestedSequenceNode : public SequenceNode {
private:
    /**
     * @brief A reference to the sequence that will be executed as a subsequence.
     */
    Sequence* target_sequence;

public:
    /**
     *
     * @param node_id The identifier of the nested sequence node that will be created
     * @param sequence A reference to the target sequence.
     */
    NestedSequenceNode(const int node_id, Sequence* sequence) : SequenceNode(node_id), target_sequence(sequence) {}

    Sequence* GetTargetSequence() const { return target_sequence; }

    /**
     *
     * @return SequenceNodeType::NESTED_SEQUENCE_NODE
     */
    SequenceNodeType GetNodeType() const override
    {
        return SequenceNodeType::NESTED_SEQUENCE_NODE;
    }
};