#pragma once
#include <memory>
#include <vector>

#include "SequenceNode.h"
#include "Transition.h"
#include "utils/SequenceState.h"

namespace AmbientCharacterBehavior {
/**
 * @brief A directed graph representing behavioral patterns that characters follow.
 *
 * The directed graph is represented through an adjacency list of transitions for each node of the sequence.
 */
class Sequence {
private:

    int32_t sequence_id;
    std::string sequence_name;

    int32_t entry_point_index;
    bool has_entry_point;

    int32_t current_node_index;
    bool has_current_node;

    /**
     * @brief the collection of sequence nodes that make up the sequence.
     * @note We store a collection of unique_ptr of SequenceNodes because the Sequence is the owner of its nodes.
     */
    std::vector<std::unique_ptr<SequenceNode>> nodes;

    /**
     * @brief The adjacency list representation of the transitions between the nodes of the sequence.
     */
    std::vector<std::vector<Transition>> transitions;

    SequenceState sequence_state;

public:
    /**
     * The constructor will initialize sequence_state to SequenceState::NORMAL
     * @throw std::invalid_argument if sequence_id < 0
     */
    explicit Sequence(int32_t sequence_id, std::string sequence_name);

    int32_t GetSequenceId() const { return sequence_id; }

    std::string GetSequenceName() const { return sequence_name; }

    const std::vector<std::unique_ptr<SequenceNode>>& GetNodes() const { return nodes; }

    int32_t GetEntryPointIndex() const { return entry_point_index; }

    bool HasEntryPoint() const { return has_entry_point; }

    int32_t GetCurrentNodeIndex() const { return current_node_index; }

    bool HasCurrentNode() const { return has_current_node; }

    const std::vector<std::vector<Transition>>& GetTransitions() const { return transitions; }

    void SetSequenceState(SequenceState state) { sequence_state = state; }

    SequenceState GetSequenceState() const { return sequence_state; }

    [[nodiscard]]
    const std::vector<Transition>& FindTransitionsFrom(int32_t node_id) const;

    /**
     * @throws std::invalid_argument if node_id < 0
     */
    void AddActionSequenceNode(int32_t node_id, int32_t action_id);

    /**
     * @throws std::invalid_argument if node_id < 0
     */
    void AddNestedSequenceNode(int32_t node_id, int32_t nested_sequence_id);

    /**
     * @throws std::invalid_argument if node_id < 0
     */
    void AddEndSequenceNode(int32_t node_id);

    [[nodiscard]]
    bool TryAddTransition(int32_t transition_id, int32_t from_node_id, int32_t to_node_id, std::vector<StateOperation> preconditions);

    [[nodiscard]]
    const SequenceNode* FindTransitionDestination(const Transition& transition) const { return FindNodeById(transition.GetDestinationNodeIndex()); }

    [[nodiscard]]
    bool TrySetEntryPoint(int32_t node_id);


    [[nodiscard]]
    const SequenceNode* FindEntryPoint() const { return has_entry_point ? FindNodeById(entry_point_index) : nullptr; }

    [[nodiscard]]
    bool TrySetCurrentNode(int32_t node_id);

    [[nodiscard]]
    const SequenceNode* FindCurrentNode() const { return has_current_node ? FindNodeById(current_node_index) : nullptr; }

    [[nodiscard]]
    const SequenceNode* FindNodeById(int32_t node_id) const { return IsNodeIdInRange(node_id) ? nodes[node_id].get() : nullptr; }

    void ResetCurrentNodeToEntry();

private:

    [[nodiscard]]
    bool IsNodeIdInRange(int32_t node_id) const { return node_id >= 0 && node_id < static_cast<int>(nodes.size()); }

};

}