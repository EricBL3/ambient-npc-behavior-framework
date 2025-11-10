#include "Sequence.h"

#include <algorithm>
#include <utility>
#include "sequence_nodes/ActionSequenceNode.h"
#include "sequence_nodes/EndSequenceNode.h"
#include "sequence_nodes/NestedSequenceNode.h"

using namespace AmbientCharacterBehavior;

Sequence::Sequence(int32_t sequence_id, std::string sequence_name) :
    sequence_id(sequence_id),
    sequence_name(std::move(sequence_name)),
    sequence_state(SequenceState::UNINITIALIZED)
{
    if (sequence_id < 0)
    {
        throw std::invalid_argument("Sequence: sequence_id cannot be negative");
    }
}

Sequence::Sequence(const Sequence &other) : sequence_id(other.sequence_id), sequence_name(other.sequence_name),
    sequence_state(SequenceState::UNINITIALIZED), entry_point_node_id(other.entry_point_node_id),
    current_node_id(other.entry_point_node_id)
{
    for (const auto&[node_id, node]: other.nodes)
    {
        if (auto action_node = dynamic_cast<ActionSequenceNode*>(node.get()))
        {
            AddActionSequenceNode(node_id, action_node->GetTargetActionId());
        }
        else if (auto nested_node = dynamic_cast<NestedSequenceNode*>(node.get()))
        {
            AddNestedSequenceNode(node_id, nested_node->GetTargetSequenceId());
        }
        else if (auto end_node = dynamic_cast<EndSequenceNode*>(node.get()))
        {
            AddEndSequenceNode(node_id);
        }
    }

    for (const auto&[from_node, transition_list] : other.transitions)
    {
        std::vector<Transition> copied_transitions;
        for (const auto& transition: transition_list)
        {
            copied_transitions.emplace_back(transition.GetTransitionId(), transition.GetDestinationNodeId(), transition.GetPreconditions());
        }

        transitions[from_node] = copied_transitions;
    }
}

std::shared_ptr<Sequence> Sequence::CreateInstance() const
{
    return std::make_shared<Sequence>(*this);
}

void Sequence::AddActionSequenceNode(int32_t node_id, int32_t action_id)
{
    nodes[node_id] = std::make_unique<ActionSequenceNode>(node_id, action_id);
}

void Sequence::AddNestedSequenceNode(int32_t node_id,int32_t nested_sequence_id)
{
    nodes[node_id] = std::make_unique<NestedSequenceNode>(node_id, nested_sequence_id);
}

void Sequence::AddEndSequenceNode(int32_t node_id)
{
    nodes[node_id] = std::make_unique<EndSequenceNode>(node_id);
}

bool Sequence::HasNode(int32_t node_id) const
{
    return nodes.find(node_id) != nodes.end();
}

SequenceNode * Sequence::FindNodeById(int32_t node_id) const
{
    auto it = nodes.find(node_id);
    return it != nodes.end() ? it->second.get() : nullptr;
}

bool Sequence::TryAddTransition(int32_t transition_id, int32_t from_node_id, int32_t to_node_id, std::vector<StateOperation> preconditions)
{
    if (HasNode(from_node_id) && HasNode(to_node_id))
    {
        transitions[from_node_id].emplace_back(transition_id, to_node_id, std::move(preconditions));
        return true;
    }

    return false;
}

std::vector<Transition> Sequence::FindTransitionsFrom(int32_t node_id) const
{
    auto it = transitions.find(node_id);
    return (it != transitions.end()) ? it->second : std::vector<Transition>{};

}

std::vector<int32_t> Sequence::GetDestinationNodeIds(int32_t from_node_id) const
{
    std::vector<int32_t> destinations;
    auto node_transitions = FindTransitionsFrom(from_node_id);
    destinations.reserve(node_transitions.size());

    for (const auto& transition : node_transitions) {
        destinations.push_back(transition.GetDestinationNodeId());
    }
    return destinations;
}

bool Sequence::IsValidTransition(int32_t from_node_id, int32_t to_node_id) const
{
    auto node_transitions = FindTransitionsFrom(from_node_id);
    return std::any_of(node_transitions.begin(), node_transitions.end(),
        [to_node_id](const Transition& t) { return t.GetDestinationNodeId() == to_node_id; });
}

bool Sequence::TrySetEntryPoint(int32_t node_id)
{
    if (HasNode(node_id))
    {
        entry_point_node_id = node_id;
        return true;
    }

    return false;
}

SequenceNode * Sequence::FindEntryPointNode() const
{
    return entry_point_node_id ? FindNodeById(entry_point_node_id.value()) : nullptr;
}

bool Sequence::TrySetCurrentNode(int32_t node_id)
{
    if (HasNode(node_id))
    {
        current_node_id = node_id;
        return true;
    }

    return false;
}

SequenceNode * Sequence::FindCurrentNode() const
{
    return current_node_id ? FindNodeById(current_node_id.value()) : nullptr;
}

void Sequence::ResetCurrentNodeToEntry()
{
    if (entry_point_node_id.has_value())
    {
        TrySetCurrentNode(entry_point_node_id.value());
    }
}

std::vector<Transition> Sequence::GetValidTransitionsFromCurrentNode() const
{
    return current_node_id ? FindTransitionsFrom(current_node_id.value()) : std::vector<Transition>{};
}
