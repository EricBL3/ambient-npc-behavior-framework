/**
 * @file Transition.cpp
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#include "Transition.h"

using namespace AmbientCharacterBehavior;

Transition::Transition(int transition_id, int to_node_index, size_t initial_preconditions_count) :
    transition_id(transition_id), to_node_index(to_node_index)
{
    if (transition_id < 0)
    {
        throw std::invalid_argument("Transition: transition_id cannot be negative");
    }

    if (to_node_index < 0)
    {
        throw std::invalid_argument("Transition: to_node_index cannot be negative");
    }

    preconditions.reserve(initial_preconditions_count);
}

void Transition::AddPrecondition(const StateOperation& precondition)
{
    preconditions.emplace_back(precondition);
}
