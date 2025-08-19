/**
 * @file Transition.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include <vector>

#include "SequenceNode.h"
#include "integration/StateOperation.h"

/**
 * @brief This class represents the conditional connection that exists between sequence nodes in a sequence.
 *
 * @see Sequence
 */
class Transition {
private:
    /**
     * @brief The unique identifier of the transition
     */
    int transition_id;

    /**
     * @brief A reference to the destination node of this transition
     */
    SequenceNode* to_node;

    /**
     * @brief The preconditions that must be satisfied for the transition to be valid
     */
    std::vector<StateOperation> preconditions;

public:
    /**
     * @param initial_preconditions_count The initial number of preconditions for this transition (default is 0).
     * @param transition_id The identifier of the transition
     */
    explicit Transition(int transition_id, size_t initial_preconditions_count = 0) :
    Transition(transition_id, nullptr, initial_preconditions_count) {}

    /**
     *
     * @param transition_id The identifier of the transition
     * @param to_node The destination node of this transition
     * @param initial_preconditions_count The initial number of preconditions for this transition (default is 0).
     */
    explicit Transition(int transition_id, SequenceNode* to_node, size_t initial_preconditions_count = 0);

    /**
     * @brief Adds a new precondition to the transition.
     * @param precondition The StateOperation that will be used as a precondition for the transition
     *
     */
    void AddPrecondition(const StateOperation& precondition);

    void SetToNode(SequenceNode* node);

    SequenceNode* GetToNode() const { return to_node; }

    const std::vector<StateOperation>& GetPreconditions() const { return preconditions; }

    int GetTransitionId() const { return transition_id; }

};
