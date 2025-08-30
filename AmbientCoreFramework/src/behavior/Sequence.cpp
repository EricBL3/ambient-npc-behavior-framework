/**
 * @file Sequence.cpp
 * @brief 
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#include "Sequence.h"
#include "ActionSequenceNode.h"
#include "EndSequenceNode.h"
#include "NestedSequenceNode.h"

using namespace AmbientCharacterBehavior;

Sequence::Sequence(int32_t sequence_id) :
    sequence_id(sequence_id),
    entry_point_index(-1),
    has_entry_point(false),
    current_node_index(-1),
    has_current_node(false),
    sequence_state(SequenceState::NORMAL),
    next_node_id(0),
    next_transition_id(0)
{
    if (sequence_id < 0)
    {
        throw std::invalid_argument("Sequence: sequence_id cannot be negative");
    }
}


const std::vector<Transition> & Sequence::GetTransitionsFromNode(int32_t node_id) const
{
    if (IsValidNodeId(node_id))
    {
        return transitions[node_id];
    }

    static const std::vector<Transition> empty_transitions;
    return empty_transitions;
}

int32_t Sequence::AddActionSequenceNode(int32_t action_id)
{

    int32_t node_id = next_node_id++;
    nodes.push_back(std::make_unique<ActionSequenceNode>(node_id, action_id));
    transitions.resize(nodes.size());

    return node_id;
}

int32_t Sequence::AddNestedSequenceNode(int32_t nested_sequence_id)
{

    int32_t node_id = next_node_id++;
    nodes.push_back(std::make_unique<NestedSequenceNode>(node_id, nested_sequence_id));
    transitions.resize(nodes.size());

    return node_id;
}

int32_t Sequence::AddEndSequenceNode()
{
    int32_t node_id = next_node_id++;
    nodes.push_back(std::make_unique<EndSequenceNode>(node_id));
    transitions.resize(nodes.size());

    return node_id;
}

int32_t Sequence::AddTransition(int32_t from_node_id, int32_t to_node_id)
{
    if (IsValidNodeId(from_node_id) && IsValidNodeId(to_node_id))
    {
        int32_t transition_id = next_transition_id++;
        transitions[from_node_id].emplace_back(transition_id, to_node_id);
        return transition_id;
    }

    return -1;
}

bool Sequence::SetEntryPoint(int32_t node_id)
{
    if (IsValidNodeId(node_id))
    {
        entry_point_index = node_id;
        has_entry_point = true;
        return true;
    }

    return false;
}

bool Sequence::SetCurrentNode(int32_t node_id)
{
    if (IsValidNodeId(node_id))
    {
        current_node_index = node_id;
        has_current_node = true;
        return true;
    }

    return false;
}

void Sequence::ResetToEntry()
{
    if (has_entry_point)
    {
        current_node_index = entry_point_index;
        has_current_node = true;
    }
}