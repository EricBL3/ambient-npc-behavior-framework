/**
 * @file Transition.cpp
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#include "Transition.h"

using namespace AmbientCharacterBehavior;

Transition::Transition(int transition_id, SequenceNode* to_node, size_t initial_preconditions_count) :
    transition_id(transition_id), to_node(to_node)
{
    if (transition_id < 0)
    {
        throw std::invalid_argument("Transition: transition_id cannot be negative");
    }

    preconditions.reserve(initial_preconditions_count);
}

void Transition::AddPrecondition(const StateOperation& precondition)
{
    preconditions.emplace_back(precondition);
}
