/**
 * @file Sequence.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include <memory>
#include <vector>

#include "Action.h"
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

    int sequence_id;

    int entry_point_index;
    bool has_entry_point;

    int current_node_index;
    bool has_current_node;

    /**
     * @brief the collection of sequence nodes that make up the sequence.
     *
     * @note We store a collection of unique_ptr of SequenceNodes because the Sequence is the owner of its nodes.
     */
    std::vector<std::unique_ptr<SequenceNode>> nodes;

    /**
     * @brief The adjacency list representation of the transitions between the nodes of the sequence.
     */
    std::vector<std::vector<Transition>> transitions;

    /**
     * @brief The lifecycle state of the sequence.
     * @see SequenceState
     */
    SequenceState sequence_state;

    /**
     * @brief Stores the value of the next node identifier. Used for auto generated IDs.
     */
    int next_node_id;

    /**
     * @brief Stores the value of the next transition identifier. Used for auto generated IDs.
     */
    int next_transition_id;

public:
    /**
     * @brief Constructs a new sequence
     *
     * The constructor will initialize sequence_state to SequenceState::NORMAL
     *
     * @param sequence_id The identifier of the sequence
     *
     * @throw std::invalid_argument if sequence_id < 0
     */
    explicit Sequence(int sequence_id);

    int GetSequenceId() const { return sequence_id; }

    const std::vector<std::unique_ptr<SequenceNode>>& GetNodes() const { return nodes; }

    int GetEntryPointIndex() const { return entry_point_index; }

    bool GetHasEntryPoint() const { return has_entry_point; }

    int GetCurrentNodeIndex() const { return current_node_index; }

    bool GetHasCurrentNode() const { return has_current_node; }

    const std::vector<std::vector<Transition>>& GetTransitions() const { return transitions; }

    /**
     *
     * @param state The new sequence state.
     */
    void SetSequenceState(SequenceState state) { sequence_state = state; }

    SequenceState GetSequenceState() const { return sequence_state; }

    /**
     * @param node_id The identifier of the node that will be searched for the transitions.
     * @return A collection with available transitions from a specific node. If no transitions are available, an empty
     * collection will be returned.
     */
    [[nodiscard]]
    const std::vector<Transition>& GetTransitionsFromNode(int node_id) const;

    /**
     * @brief Adds a new action sequence node to the sequence.
     * @param action_id The identifier of the action that will be referenced by the node
     * @return The node_id of the created sequence node.
     *
     */
    [[nodiscard]]
    int AddActionSequenceNode(int action_id);

    /**
     * @brief Adds a new nested sequence node to the sequence.
     * @param nested_sequence_id The identifier of the sequence that will be referenced by the node
     * @return The node_id of the created sequence node.
     *
     */
    [[nodiscard]]
    int AddNestedSequenceNode(int nested_sequence_id);

    /**
     * @brief Adds a new end sequence node to the sequence.
     * @return The node_id of the created sequence node.
     */
    [[nodiscard]]
    int AddEndSequenceNode();

    /**
     * @brief Adds a new transition to the sequence.
     * @param from_node_id The node id from which the transition starts.
     * @param to_node_id The node id of the destination of the transition.
     *
     * @return The transition_id of the created transition (or -1 if it couldn't be added).
     *
     * @note The transition will be empty which means that the to_node and preconditions must still be defined.
     */
    [[nodiscard]]
    int AddTransition(int from_node_id, int to_node_id);

    /**
     * @brief
     * @param transition The transition from which we want to get the destination node.
     * @return A reference to the sequence node that the transition points to or nullptr if the destination node doesn't exist.
     */
    [[nodiscard]]
    const SequenceNode* GetTransitionToNode(const Transition& transition) const { return GetNode(transition.GetToNodeIndex()); }

    /**
     * Sets the entry point for the sequence.
     * @param node_id The identifier of the node that will be set up as the entry point.
     * @return True if the entry point could be set, false if not (due to an invalid node_id)
     */
    [[nodiscard]]
    bool SetEntryPoint(int node_id);

    /**
     *
     * @return The sequence node that represents the entry point of the sequence or a nullptr if there isn't one.
     */
    [[nodiscard]]
    const SequenceNode* GetEntryPoint() const { return has_entry_point ? GetNode(entry_point_index) : nullptr; }

    /**
     * Sets the current node for execution in the sequence.
     * @param node_id The identifier of the node that will be set up as the current node.
     * @return True if the current node could be set, false if not (due to an invalid node_id)
     */
    [[nodiscard]]
    bool SetCurrentNode(int node_id);

    /**
     *
     * @return The sequence node that represents the current node of the sequence or a nullptr if there isn't one.
     */
    [[nodiscard]]
    const SequenceNode* GetCurrentNode() const { return has_current_node ? GetNode(current_node_index) : nullptr; }

    /**
     * @param node_id the identifier of the node to get
     * @return A reference to the sequence node or nullptr if the node_id is not in the sequence.
     */
    [[nodiscard]]
    const SequenceNode* GetNode(int node_id) const { return IsValidNodeId(node_id) ? nodes[node_id].get() : nullptr; }

    /**
     * @brief Resets the current node index to the entry node index for handling failure recovery.
     */
    void ResetToEntry();

private:
    /**
     * Checks if a sequence node identifier is part of the nodes in the sequence
     * @param node_id The identifier of the node to check
     * @return True if the node_id exists, false if not (or invalid)
     */
    [[nodiscard]]
    bool IsValidNodeId(int node_id) const { return node_id >= 0 && node_id < static_cast<int>(nodes.size()); }

};

}