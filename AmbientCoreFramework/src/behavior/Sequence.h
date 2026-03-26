#pragma once
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "sequence_nodes/SequenceNode.h"
#include "Transition.h"
#include "enums/SequenceState.h"

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

    std::optional<int32_t> entry_point_node_id;
    std::optional<int32_t> current_node_id;

    /**
     * @note We store a collection of unique_ptr of SequenceNodes because the Sequence is the owner of its nodes.
     */
    std::unordered_map<int32_t, std::unique_ptr<SequenceNode>> nodes;

    /**
     * @brief The adjacency list representation of the transitions between the nodes of the sequence.
     */
    std::unordered_map<int32_t, std::vector<Transition>> transitions;

    SequenceState sequence_state;

public:
    /**
     * The constructor will initialize sequence_state to SequenceState::NORMAL
     * @throw std::invalid_argument if sequence_id < 0
     */
    explicit Sequence(int32_t sequence_id, std::string sequence_name);

    Sequence(const Sequence& other);

    std::shared_ptr<Sequence> CreateInstance() const;

    int32_t GetSequenceId() const { return sequence_id; }
    std::string GetSequenceName() const { return sequence_name; }
    SequenceState GetSequenceState() const { return sequence_state; }
    void SetSequenceState(SequenceState state) { sequence_state = state; }

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
    bool HasNode(int32_t node_id) const;

    [[nodiscard]]
    SequenceNode* FindNodeById(int32_t node_id) const;

    size_t GetNodeCount() const { return nodes.size(); }

    bool TryAddTransition(int32_t transition_id, int32_t from_node_id, int32_t to_node_id,
        std::unordered_map<StateOperationTarget, std::vector<StateOperation>> preconditions_by_target);

    [[nodiscard]]
    std::vector<Transition> FindTransitionsFrom(int32_t node_id) const;

    std::vector<int32_t> GetDestinationNodeIds(int32_t from_node_id) const;

    [[nodiscard]]
    bool IsValidTransition(int32_t from_node_id, int32_t to_node_id) const;

    bool TrySetEntryPoint(int32_t node_id);

    bool HasEntryPoint() const { return entry_point_node_id.has_value(); }

    int32_t GetEntryPointNodeId() const { return entry_point_node_id.value_or(-1); }

    [[nodiscard]]
    SequenceNode* FindEntryPointNode() const;

    bool TrySetCurrentNode(int32_t node_id);

    bool HasCurrentNode() const { return current_node_id.has_value(); }

    int32_t GetCurrentNodeId() const { return current_node_id.value_or(-1); }

    [[nodiscard]]
    SequenceNode* FindCurrentNode() const;

    void ResetCurrentNodeToEntry();

    std::vector<Transition> GetValidTransitionsFromCurrentNode() const;

    [[nodiscard]]
    const SequenceNode* FindTransitionDestination(const Transition& transition) const
    {
        return FindNodeById(transition.GetDestinationNodeId());
    }

};

}