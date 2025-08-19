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

const std::vector<Transition> & Sequence::GetTransitionsFromNode(int node_id) const
{
    if (IsValidNodeId(node_id))
    {
        return transitions[node_id];
    }

    static const std::vector<Transition> empty_transitions;
    return empty_transitions;
}

int Sequence::AddActionSequenceNode(Action *action)
{
    int node_id = next_node_id++;
    nodes.push_back(std::make_unique<ActionSequenceNode>(node_id, action));
    transitions.resize(nodes.size());

    return node_id;
}

int Sequence::AddNestedSequenceNode(Sequence *sequence)
{
    int node_id = next_node_id++;
    nodes.push_back(std::make_unique<NestedSequenceNode>(node_id, sequence));
    transitions.resize(nodes.size());

    return node_id;
}

int Sequence::AddEndSequenceNode()
{
    int node_id = next_node_id++;
    nodes.push_back(std::make_unique<EndSequenceNode>(node_id));
    transitions.resize(nodes.size());

    return node_id;
}

bool Sequence::AddTransition(int from_node_id)
{
    if (IsValidNodeId(from_node_id))
    {
        int transition_id = next_transition_id++;
        transitions[from_node_id].push_back(Transition(transition_id));
        return true;
    }

    return false;
}

bool Sequence::SetEntryPoint(int node_id)
{
    if (IsValidNodeId(node_id))
    {
        entry_point_index = node_id;
        has_entry_point = true;
        return true;
    }

    return false;
}

const SequenceNode * Sequence::GetEntryPoint() const
{
    if (has_entry_point && IsValidNodeId(entry_point_index))
    {
        return nodes[entry_point_index].get();
    }

    return nullptr;
}

bool Sequence::SetCurrentNode(int node_id)
{
    if (IsValidNodeId(node_id))
    {
        current_node_index = node_id;
        has_current_node = true;
        return true;
    }

    return false;
}

const SequenceNode * Sequence::GetCurrentNode() const
{
    if (has_current_node && IsValidNodeId(current_node_index))
    {
        return nodes[current_node_index].get();
    }

    return nullptr;
}

void Sequence::ResetToEntry()
{
    if (has_entry_point)
    {
        current_node_index = entry_point_index;
        has_current_node = true;
    }
}

bool Sequence::IsValidNodeId(int node_id) const
{
    return node_id >= 0 && node_id < static_cast<int>(nodes.size());
}
