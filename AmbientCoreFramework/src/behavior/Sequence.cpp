#include "Sequence.h"

#include <utility>
#include "ActionSequenceNode.h"
#include "EndSequenceNode.h"
#include "NestedSequenceNode.h"

using namespace AmbientCharacterBehavior;

Sequence::Sequence(int32_t sequence_id, std::string sequence_name) :
    sequence_id(sequence_id),
    sequence_name(std::move(sequence_name)),
    entry_point_index(-1),
    has_entry_point(false),
    current_node_index(-1),
    has_current_node(false),
    sequence_state(SequenceState::NORMAL)
{
    if (sequence_id < 0)
    {
        throw std::invalid_argument("Sequence: sequence_id cannot be negative");
    }
}

const std::vector<Transition> & Sequence::FindTransitionsFrom(int32_t node_id) const
{
    if (IsNodeIdInRange(node_id))
    {
        return transitions[node_id];
    }

    return {};
}

void Sequence::AddActionSequenceNode(int32_t node_id, int32_t action_id)
{
    nodes.push_back(std::make_unique<ActionSequenceNode>(node_id, action_id));
    transitions.resize(nodes.size());
}

void Sequence::AddNestedSequenceNode(int32_t node_id,int32_t nested_sequence_id)
{
    nodes.push_back(std::make_unique<NestedSequenceNode>(node_id, nested_sequence_id));
    transitions.resize(nodes.size());
}

void Sequence::AddEndSequenceNode(int32_t node_id)
{
    nodes.push_back(std::make_unique<EndSequenceNode>(node_id));
    transitions.resize(nodes.size());
}

bool Sequence::TryAddTransition(int32_t transition_id, int32_t from_node_id, int32_t to_node_id, std::vector<StateOperation> preconditions)
{
    if (IsNodeIdInRange(from_node_id) && IsNodeIdInRange(to_node_id))
    {
        transitions[from_node_id].emplace_back(transition_id, to_node_id, std::move(preconditions));
        return true;
    }

    return false;
}

bool Sequence::TrySetEntryPoint(int32_t node_id)
{
    if (IsNodeIdInRange(node_id))
    {
        entry_point_index = node_id;
        has_entry_point = true;
        return true;
    }

    return false;
}

bool Sequence::TrySetCurrentNode(int32_t node_id)
{
    if (IsNodeIdInRange(node_id))
    {
        current_node_index = node_id;
        has_current_node = true;
        return true;
    }

    return false;
}

void Sequence::ResetCurrentNodeToEntry()
{
    if (has_entry_point)
    {
        current_node_index = entry_point_index;
        has_current_node = true;
    }
}