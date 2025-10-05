#include "BehavioralEntity.h"

#include <algorithm>

#include "behavior/ActionSequenceNode.h"
#include "behavior/NestedSequenceNode.h"
#include "services/composition/ServiceBuilder.h"

using namespace AmbientCharacterBehavior;

bool BehavioralEntity::CanUpdate() const
{
    if (sequences.empty())
    {
        return !is_processing;
    }

    return !is_processing && sequences.top()->GetSequenceState() != SequenceState::WAITING_FOR_ACTION;
}

void BehavioralEntity::SetMainSequence(const std::shared_ptr<Sequence> &new_sequence)
{
    if (new_sequence && new_sequence != main_sequence)
    {
        main_sequence = new_sequence->CreateInstance();
    }
}

void BehavioralEntity::AddFallbackSequence(const std::shared_ptr<Sequence> &new_sequence)
{
    if (new_sequence && !HasFallbackSequence(new_sequence->GetSequenceId()))
    {
        fallback_sequences.emplace_back(new_sequence->CreateInstance());
    }
}

bool BehavioralEntity::HasFallbackSequence(int32_t sequence_id) const
{
    return std::any_of(fallback_sequences.begin(), fallback_sequences.end(),
                       [sequence_id](const std::shared_ptr<Sequence>& seq) {
                           return seq && seq->GetSequenceId() == sequence_id;
                       });
}

std::shared_ptr<Sequence> BehavioralEntity::GetFallbackSequenceById(int32_t sequence_id) const
{
    auto iterator = std::find_if(fallback_sequences.begin(), fallback_sequences.end(),
                           [sequence_id](const std::shared_ptr<Sequence>& seq) {
                               return seq && seq->GetSequenceId() == sequence_id;
                           });

    if (iterator == fallback_sequences.end())
    {
        logger.LogWarning("Sequence with id: " + std::to_string(sequence_id) + " is not in the fallback sequences",
         "BehavioralEntity");
        return nullptr;
    }

    return *iterator;
}

void BehavioralEntity::AddInterruptionHandler(int32_t interruption_id, const std::shared_ptr<Sequence> &handler)
{
    if (handler)
    {
        interruption_handlers.insert_or_assign(interruption_id, handler->CreateInstance());
    }
}

std::shared_ptr<Sequence> BehavioralEntity::FindInterruptionHandler(int32_t interruption_id) const
{
    auto iterator = interruption_handlers.find(interruption_id);
    if (iterator == interruption_handlers.end())
    {
        logger.LogWarning("FrameworkEntity[" + std::to_string(entity_id) + "]: Interruption id: " +
            std::to_string(interruption_id) + " not found", "BehavioralEntity");

        return nullptr;
    }

    return iterator->second;
}

void BehavioralEntity::ExecuteCurrentSequence()
{
    is_processing = true;

    if (sequences.empty())
    {
        HandleEmptySequences();
        return;
    }

    logger.LogInfo("Executing sequence for entity: " + std::to_string(entity_id), "BehavioralEntity");

    switch (sequences.top()->GetSequenceState())
    {
        case SequenceState::UNINITIALIZED:
            HandleSequenceStartup();
            break;
        case SequenceState::PROCESSING_NODE:
            ExecuteCurrentNode();
            break;
        case SequenceState::IN_SUBSEQUENCE:
            HandleSubsequenceCompletion();
            break;
        case SequenceState::WAITING_FOR_ACTION:
            logger.LogInfo("Waiting for character '" + std::to_string(entity_id) + "' to complete action with id: " +
                std::to_string(current_action_id), "BehavioralEntity");
            break;
        case SequenceState::NODE_EXECUTED:
            HandleNodeExecutionCompletion();
            break;
        case SequenceState::FAILED:
            HandleSequenceFailure();
            break;
        case SequenceState::INTERRUPTED:
            HandleInterruptionRecovery();
            break;
    }

    is_processing = false;
}

void BehavioralEntity::HandleEmptySequences()
{
    if (main_sequence == nullptr)
    {
        logger.LogError("character with id: " + std::to_string(entity_id) + " does not have a valid main sequence.",
            "BehavioralEntity");

        // is processing is turned true to prevent the character from receiving future updates in this invalid state.
        is_processing = true;
        return;
    }

    sequences.emplace(main_sequence);
    sequences.top()->SetSequenceState(SequenceState::UNINITIALIZED);
    is_processing = false;
}


void BehavioralEntity::HandleSequenceStartup()
{
    logger.LogInfo("Handling sequence startup for entity: " + std::to_string(entity_id), "BehavioralEntity");
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
    sequences.top()->ResetCurrentNodeToEntry();
}

void BehavioralEntity::ExecuteCurrentNode()
{
    auto current_node = sequences.top()->FindCurrentNode();
    auto current_node_type = current_node->GetNodeType();

    if (current_node_type == SequenceNodeType::ACTION_NODE)
    {
        ExecuteActionNode(current_node);
    }
    else if (current_node_type == SequenceNodeType::NESTED_SEQUENCE_NODE)
    {
        ExecuteNestedSequenceNode(current_node);
    }
    else if (current_node_type == SequenceNodeType::END_SEQUENCE_NODE)
    {
        ExecuteEndSequenceNode(current_node);
    }
    else
    {
        logger.LogWarning("The current node type is not supported", "BehavioralEntity");
        HandleSequenceFailure();
    }
}

void BehavioralEntity::ExecuteEndSequenceNode(const SequenceNode* current_node)
{
    sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
    sequences.pop();
}

void BehavioralEntity::ExecuteNestedSequenceNode(const SequenceNode* current_node)
{
    auto nested_sequence_node = dynamic_cast<const NestedSequenceNode*>(current_node);
    if (!nested_sequence_node)
    {
        logger.LogError("The current node type is not of type nested sequence node",
            "BehavioralEntity");

        HandleSequenceFailure();
        return;
    }

    sequences.top()->SetSequenceState(SequenceState::IN_SUBSEQUENCE);
    std::shared_ptr<Sequence> nested_sequence = content_provider.GetSequenceById(nested_sequence_node->GetTargetSequenceId());
    if (!nested_sequence)
    {
        logger.LogError("Could not find nested sequence with id: " + std::to_string(nested_sequence_node->GetTargetSequenceId()),
            "BehavioralEntity");

        HandleSequenceFailure();
        return;
    }

    sequences.emplace(nested_sequence);
}

void BehavioralEntity::HandleSubsequenceCompletion()
{
    //todo: implement
}

void BehavioralEntity::ExecuteActionNode(const SequenceNode* current_node)
{
    auto action_sequence_node = dynamic_cast<const ActionSequenceNode*>(current_node);
    if (!action_sequence_node)
    {
        logger.LogError("The current node type is not of type action sequence node",
            "BehavioralEntity");

        HandleSequenceFailure();
        return;
    }

    std::shared_ptr<Action> action = content_provider.GetActionById(action_sequence_node->GetTargetActionId());
    if (!action)
    {
        logger.LogError("Could not find action with id: " + std::to_string(action_sequence_node->GetTargetActionId()),
            "BehavioralEntity");

        HandleSequenceFailure();
        return;
    }

    FrameworkEntity* target_entity = nullptr;
    void* target_entity_handle = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = GetActionTargetEntity(action);

        if (!target_entity)
        {
            logger.LogWarning("No valid entities found for action " + std::to_string(action->GetActionId()) +
                " - triggering sequence failure","BehavioralEntity");

            HandleSequenceFailure();
            return;
        }

        target_entity_handle = target_entity->GetEntityHandle();
    }

    // Apply immediate effects
    ApplyActionEffects(action->GetImmediateEffects(), target_entity);

    // Start action
    current_action_id = action->GetActionId();
    current_action_token++;

    logger.LogInfo("Calling start character action for entity: " + std::to_string(entity_id) + " with action id: " +
        std::to_string(current_action_id) + " and token: " + std::to_string(current_action_token), "BehavioralEntity");

    start_character_action_provider.StartCharacterAction(entity_handle, current_action_id, current_action_token,
        target_entity_handle);

    sequences.top()->SetSequenceState(SequenceState::WAITING_FOR_ACTION);

    //todo: add some internal timer for action completion
}

FrameworkEntity* BehavioralEntity::GetActionTargetEntity(const std::shared_ptr<Action>& action)
{
    FrameworkEntity* target_entity = nullptr;

    // Evaluate entities
    std::vector<FrameworkEntity*> entities = entity_query.GetEntitiesSupportingAction(action->GetActionId());

    // Filter to have only the entities that can be done (precondition satisfaction)
    std::vector<int32_t> entity_ids;
    for (auto entity : entities)
    {
        auto can_do_action = true;

        for (const auto& precondition : action->GetPreconditions())
        {
            if (!state_operation_evaluator.ProcessStateOperation(precondition, entity))
            {
                can_do_action = false;
                break;
            }
        }

        if (can_do_action)
        {
            entity_ids.push_back(entity->GetEntityId());
        }
    }

    // Select best entity
    auto selected_entity_id = memory.GetLeastRecentlyUsedEntityIdForAction(action->GetActionId(), entity_ids);
    target_entity = entity_query.GetEntityFromId(selected_entity_id);

    // Set current action target id
    if (target_entity)
    {
        current_action_target_id = target_entity->GetEntityId();
    }

    return target_entity;
}

void BehavioralEntity::ApplyActionEffects(const std::vector<StateOperation> & effects, FrameworkEntity* target_entity)
{
    for (const auto& effect: effects)
    {
        state_operation_evaluator.ProcessStateOperation(effect, target_entity);
    }
}

void BehavioralEntity::CompleteAction(int32_t action_id, int64_t action_token)
{
    if (CompletedCurrentAction(action_id, action_token))
    {
        logger.LogInfo("entity with id: " + std::to_string(entity_id) + " has completed action with id: " +
            std::to_string(action_id) + " and token: " + std::to_string(action_token) ,"BehavioralEntity");

        //todo: Apply completion effects

        //todo: Update action memory

        //todo: Mark node as completed

        if (!sequences.empty())
        {
            sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
            logger.LogInfo("entity with id: " + std::to_string(entity_id) + " is now in NODE_EXECUTED step",
                "BehavioralEntity");
        }
        else
        {
            HandleEmptySequences();
        }

        is_processing = false;
    }
}

bool BehavioralEntity::CompletedCurrentAction(int32_t action_id, int64_t action_token) const
{
    if (action_token != current_action_token || action_id != current_action_id)
    {
        logger.LogWarning("The completed action with id: " + std::to_string(action_id) + " and token: " +
            std::to_string(action_token) + " is not the same as the currently executing action for entity with id: " +
            std::to_string(entity_id),"BehavioralEntity");

        return false;
    }

    return true;
}

void BehavioralEntity::HandleNodeExecutionCompletion()
{
    // EvaluateTransitions();
    // SelectBestTransition();
    // memory.UpdateTransitionMemory();
    // sequences.top()->SetSequenceState();
    // sequences.top()->GetCurrentNode().ResetCompletion();
    logger.LogInfo("Completed node execution for entity " + std::to_string(entity_id) + ". Will reset sequence to "
        "PROCESSING_NODE state", "BehavioralEntity");

    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
}

void BehavioralEntity::HandleSequenceFailure()
{
}

void BehavioralEntity::HandleInterruptionRecovery()
{
}

void BehavioralEntity::ProcessInterruption(int32_t interruption_id)
{
    logger.LogInfo("Processing interruption with id: " + std::to_string(interruption_id),
        "BehavioralEntity");
    //TODO: ADD FULL IMPLEMENTATION
}