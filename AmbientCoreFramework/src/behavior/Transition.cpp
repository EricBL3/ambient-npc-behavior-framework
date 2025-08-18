/**
 * @file Transition.cpp
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#include "Transition.h"

Transition::Transition(int transition_id, SequenceNode* to_node, size_t initial_preconditions_count) :
    transition_id(transition_id), to_node(to_node)
{
    preconditions.reserve(initial_preconditions_count);
}

void Transition::SetToNode(SequenceNode* node)
{
    this->to_node = node;
}

void Transition::AddPrecondition(const StateOperation& precondition)
{
    preconditions.emplace_back(precondition);
}

const std::vector<StateOperation>& Transition::GetPreconditions() const
{
    return preconditions;
}

SequenceNode* Transition::GetToNode() const
{
    return to_node;
}