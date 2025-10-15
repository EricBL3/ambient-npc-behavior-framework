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

    ExecuteSequenceStep(sequences.top()->GetSequenceState());

    is_processing = false;
}

void BehavioralEntity::ExecuteSequenceStep(SequenceState sequence_state)
{
    logger.LogInfo("Executing sequence state " + ToString(sequence_state) + " for entity: " +
        std::to_string(entity_id), "BehavioralEntity");

    switch (sequence_state)
    {
        case SequenceState::UNINITIALIZED:
            HandleSequenceStartup();
            break;
        case SequenceState::PROCESSING_NODE:
            ProcessCurrentNode();
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
    logger.LogInfo("Handling sequence startup for entity: " + std::to_string(entity_id),
        "BehavioralEntity");

    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
    sequences.top()->ResetCurrentNodeToEntry();
}

void BehavioralEntity::ProcessCurrentNode()
{
    logger.LogInfo("Processing current node for entity: " + std::to_string(entity_id),
        "BehavioralEntity");

    if (const auto current_node = TryGetCurrentNode())
    {
        ExecuteCurrentNode(current_node);
    }
}

SequenceNode* BehavioralEntity::TryGetCurrentNode()
{
    auto current_node = sequences.top()->FindCurrentNode();
    if (!current_node)
    {
        logger.LogError("Could not find the current node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()),
            "BehavioralEntity");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
    }

    return current_node;
}

void BehavioralEntity::ExecuteCurrentNode(const SequenceNode* current_node)
{
    auto current_node_type = current_node->GetNodeType();
    if (current_node_type == SequenceNodeType::ACTION_NODE)
    {
        logger.LogInfo("Will execute action node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()) +
            " for entity with id: " + std::to_string(entity_id), "BehavioralEntity");

        ExecuteActionNode(current_node);
    }
    else if (current_node_type == SequenceNodeType::NESTED_SEQUENCE_NODE)
    {
        logger.LogInfo("Will execute nested sequence node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()) +
            " for entity with id: " + std::to_string(entity_id), "BehavioralEntity");

        ExecuteNestedSequenceNode(current_node);
    }
    else if (current_node_type == SequenceNodeType::END_SEQUENCE_NODE)
    {
        logger.LogInfo("Will execute end node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()) +
            " for entity with id: " + std::to_string(entity_id), "BehavioralEntity");

        ExecuteEndSequenceNode(current_node);
    }
    else
    {
        logger.LogWarning("The current node type is not supported", "BehavioralEntity");
        sequences.top()->SetSequenceState(SequenceState::FAILED);
    }
}

void BehavioralEntity::ExecuteEndSequenceNode(const SequenceNode* current_node)
{
    logger.LogInfo("Reached end of sequence for entity " + std::to_string(entity_id),
                   "BehavioralEntity");

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

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    sequences.top()->SetSequenceState(SequenceState::IN_SUBSEQUENCE);
    std::shared_ptr<Sequence> nested_sequence = content_provider.GetSequenceById(nested_sequence_node->GetTargetSequenceId());
    if (!nested_sequence)
    {
        logger.LogError("Could not find nested sequence with id: " + std::to_string(nested_sequence_node->GetTargetSequenceId()),
            "BehavioralEntity");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    sequences.emplace(nested_sequence);
}

void BehavioralEntity::HandleSubsequenceCompletion()
{
    logger.LogInfo("Finished running subsequence for entity" + std::to_string(entity_id),
        "BehavioralEntity");

    sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
    HandleNodeExecutionCompletion();
}

void BehavioralEntity::ExecuteActionNode(const SequenceNode* current_node)
{
    // Lookup action
    auto action_sequence_node = dynamic_cast<const ActionSequenceNode*>(current_node);
    if (!action_sequence_node)
    {
        logger.LogError("The current node type is not of type action sequence node",
            "BehavioralEntity");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    std::shared_ptr<Action> action = content_provider.GetActionById(action_sequence_node->GetTargetActionId());
    if (!action)
    {
        logger.LogError("Could not find action with id: " + std::to_string(action_sequence_node->GetTargetActionId()),
            "BehavioralEntity");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    // Acquire target entity
    FrameworkEntity* target_entity = nullptr;
    void* target_entity_handle = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = GetActionTargetEntity(action);

        if (!target_entity)
        {
            logger.LogWarning("No valid entities found for action " + std::to_string(action->GetActionId()) +
                " - triggering sequence failure","BehavioralEntity");

            sequences.top()->SetSequenceState(SequenceState::FAILED);
            return;
        }

        target_entity_handle = target_entity->GetEntityHandle();
    }

    InitiateActionExecution(action, target_entity, target_entity_handle);
}

void BehavioralEntity::InitiateActionExecution(const std::shared_ptr<Action>& action, FrameworkEntity* target_entity,
    void* target_entity_handle, bool apply_immediate_effects)
{
    // Apply immediate effects
    if (apply_immediate_effects)
    {
        ApplyActionEffects(action->GetImmediateEffects(), target_entity);
    }

    // Start action
    if (target_entity)
    {
        current_action_target_id = target_entity->GetEntityId();
    }

    current_action_id = action->GetActionId();
    current_action_token++;

    logger.LogInfo("Calling start character action for entity: " + std::to_string(entity_id) + " with action id: " +
        std::to_string(current_action_id) + " and token: " + std::to_string(current_action_token), "BehavioralEntity");

    start_character_action_provider.StartCharacterAction(entity_handle, current_action_id, current_action_token,
        action->GetMaxDuration(), target_entity_handle);

    sequences.top()->SetSequenceState(SequenceState::WAITING_FOR_ACTION);
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
            FrameworkEntity* precondition_entity = nullptr;
            switch (precondition.GetTarget())
            {
                case StateOperationTarget::SELF:
                    precondition_entity = this;
                    break;
                case StateOperationTarget::ENTITY:
                    precondition_entity = entity;
                    break;
                default:
                    break;
            }

            if (!state_operation_evaluator.ProcessStateOperation(precondition, precondition_entity))
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

    return target_entity;
}

void BehavioralEntity::ApplyActionEffects(const std::vector<StateOperation> & effects, FrameworkEntity* target_entity)
{
    for (const auto& effect: effects)
    {
        FrameworkEntity* effect_entity = nullptr;
        switch (effect.GetTarget())
        {
            case StateOperationTarget::SELF:
                effect_entity = this;
                break;
            case StateOperationTarget::ENTITY:
                effect_entity = target_entity;
                break;
            default:
                break;
        }

        state_operation_evaluator.ProcessStateOperation(effect, effect_entity);
    }

    logger.LogInfo("Applied " + std::to_string(effects.size()) + " effects for action processing of " +
        " entity " + std::to_string(entity_id), "BehavioralEntity");
}

void BehavioralEntity::CompleteAction(int32_t action_id, int64_t action_token)
{
    if (CompletedCurrentAction(action_id, action_token))
    {
        logger.LogInfo("entity with id: " + std::to_string(entity_id) + " has completed action with id: " +
            std::to_string(action_id) + " and token: " + std::to_string(action_token) ,"BehavioralEntity");

        // Apply completion effects
        auto action = content_provider.GetActionById(action_id);
        if (!action)
        {
            logger.LogError("Could not find action with id" + std::to_string(action_id),
                "BehavioralEntity");

            sequences.top()->SetSequenceState(SequenceState::FAILED);
            return;
        }

        FrameworkEntity* target_entity = nullptr;
        if (action->GetRequiresTargetEntity())
        {
            target_entity = entity_query.GetEntityFromId(current_action_target_id);
            if (!target_entity)
            {
                logger.LogError("Could not find target entity with id" + std::to_string(current_action_target_id),
                    "BehavioralEntity");

                sequences.top()->SetSequenceState(SequenceState::FAILED);
                is_processing = false;
                return;
            }
        }

        ApplyActionEffects(action->GetCompletionEffects(), target_entity);

        memory.UpdateActionMemory(action_id, current_action_target_id, time_manager.GetCurrentTime());

        // Reset current_action_target_id and current_action_id to invalid value.
        current_action_target_id = -1;
        current_action_id = -1;

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
    auto current_node = sequences.top()->FindCurrentNode();
    if (!current_node)
    {
        logger.LogError("Could not find the current node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()),
            "BehavioralEntity");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    current_node->MarkAsCompleted();

    // Evaluate transitions
    std::vector<Transition> transitions = sequences.top()->GetValidTransitionsFromCurrentNode();

    // Filter to have only the nodes that can be transitioned to (precondition satisfaction)
    std::vector<int32_t> node_ids;
    for (const auto& transition : transitions)
    {
        auto can_do_transition = true;

        for (const auto& precondition : transition.GetPreconditions())
        {
            FrameworkEntity* precondition_entity = nullptr;
            switch (precondition.GetTarget())
            {
                case StateOperationTarget::SELF:
                    precondition_entity = this;
                    break;
                default:
                    break;
            }

            if (!state_operation_evaluator.ProcessStateOperation(precondition, precondition_entity))
            {
                can_do_transition = false;
                break;
            }
        }

        if (can_do_transition)
        {
            node_ids.push_back(transition.GetDestinationNodeId());
        }
    }

    // Select best node to transition to
    auto selected_node_id = memory.GetLeastRecentlyVisitedNodeId(node_ids);
    if (!sequences.top()->TrySetCurrentNode(selected_node_id))
    {
        logger.LogError("Could not set current node with id: " + std::to_string(selected_node_id),
            "BehavioralEntity");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    memory.UpdateTransitionMemory(selected_node_id, time_manager.GetCurrentTime());
    sequences.top()->FindCurrentNode()->ResetCompletion();
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
}

void BehavioralEntity::HandleSequenceFailure()
{
    logger.LogInfo("Handling sequence failure for entity: " + std::to_string(entity_id),
        "BehavioralEntity");

    memory.ClearSequenceInterruptionMemories(sequences.top()->GetSequenceId());
    sequences.pop();

    if (fallback_sequences.empty())
    {
        logger.LogError("No fallback sequences available for entity " +
                       std::to_string(entity_id), "BehavioralEntity");

        return;
    }

    auto fallback_sequence = fallback_sequences[rand() % fallback_sequences.size()];
    logger.LogInfo("Entity with id: " + std::to_string(entity_id) + " will now follow fallback sequence with id: " +
        std::to_string(fallback_sequence->GetSequenceId()), "BehavioralEntity");

    sequences.push(fallback_sequence);
}

void BehavioralEntity::HandleInterruptionRecovery()
{
    // Check if sequence was executing action
    if (current_action_id >= 0)
    {
        auto action = content_provider.GetActionById(current_action_id);
        if (!action)
        {
            logger.LogError("Could not find action with id: " + std::to_string(current_action_id),
                "HandleInterruptionRecovery");

            HandleSequenceFailure();
            return;
        }

        if (action->GetInterruptionBehavior() == InterruptionBehaviorType::RESUMABLE)
        {
            AttemptActionResumption();
            return;
        }
    }

    logger.LogInfo("The interrupted sequence for entity " + std::to_string(entity_id) + " does not require action resumption",
        "HandleInterruptionRecovery");
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
}

void BehavioralEntity::AttemptActionResumption()
{
    auto interruption_memory = memory.FindInterruptionMemory(current_action_id, sequences.top()->GetSequenceId(),
                sequences.top()->GetCurrentNodeId());

    if (!interruption_memory)
    {
        logger.LogInfo("No interruption memory exists for action " + std::to_string(current_action_id) +
            " for entity " + std::to_string(entity_id), "AttemptActionResumption");

        sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
        return;
    }

    auto action = content_provider.GetActionById(interruption_memory->GetInterruptedActionId());
    if (!action)
    {
        logger.LogError("Could not find interrupted action with id: " +
            std::to_string(interruption_memory->GetInterruptedActionId()),
            "AttemptActionResumption");

        memory.RemoveInterruptionMemory(interruption_memory);
        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    if (ValidateResumptionContext(action, interruption_memory->GetInterruptedTargetEntityId()))
    {
        logger.LogInfo("Resuming action " + std::to_string(action->GetActionId()) +
            " with saved target entity " +
            std::to_string(interruption_memory->GetInterruptedTargetEntityId()),
            "AttemptActionResumption");

        ResumeActionWithSavedContext(action, interruption_memory);

        memory.RemoveInterruptionMemory(interruption_memory);
        return;
    }

    logger.LogInfo("Resumption context invalid for action " + std::to_string(action->GetActionId()) +
        ", attempting fresh execution", "AttemptActionResumption");

    memory.RemoveInterruptionMemory(interruption_memory);
    ExecuteActionNode(sequences.top()->FindCurrentNode());
}

bool BehavioralEntity::ValidateResumptionContext(const std::shared_ptr<Action>& action, int32_t target_entity_id)
{
    // If action doesn't require target entity, context is always valid
    if (!action->GetRequiresTargetEntity())
    {
        return true;
    }

    auto target_entity = entity_query.GetEntityFromId(target_entity_id);
    if (!target_entity)
    {
        logger.LogInfo("Target entity " + std::to_string(target_entity_id) +
            " no longer exists", "ValidateResumptionContext");
        return false;
    }

    if (!target_entity->SupportsAction(action->GetActionId()))
    {
        logger.LogInfo("Target entity " + std::to_string(target_entity_id) +
            " no longer supports action " + std::to_string(action->GetActionId()),
            "ValidateResumptionContext");
        return false;
    }

    for (const auto& precondition : action->GetPreconditions())
    {
        FrameworkEntity* precondition_entity = nullptr;
        switch (precondition.GetTarget())
        {
            case StateOperationTarget::SELF:
                precondition_entity = this;
                break;
            case StateOperationTarget::ENTITY:
                precondition_entity = target_entity;
                break;
            default:
                break;
        }

        if (!state_operation_evaluator.ProcessStateOperation(precondition, precondition_entity))
        {
            logger.LogInfo("Precondition no longer satisfied for action " +
                std::to_string(action->GetActionId()), "ValidateResumptionContext");
            return false;
        }
    }

    return true;
}

void BehavioralEntity::ResumeActionWithSavedContext(const std::shared_ptr<Action>& action, const InterruptionMemory* interruption_memory)
{
    FrameworkEntity* target_entity = nullptr;
    void* target_entity_handle = nullptr;

    if (action->GetRequiresTargetEntity())
    {
        target_entity = entity_query.GetEntityFromId(
            interruption_memory->GetInterruptedTargetEntityId());

        if (!target_entity)
        {
            logger.LogError("Target entity vanished during resumption",
                "ResumeActionWithSavedContext");
            sequences.top()->SetSequenceState(SequenceState::FAILED);
            return;
        }

        target_entity_handle = target_entity->GetEntityHandle();
    }

    InitiateActionExecution(action, target_entity, target_entity_handle, false);
}

void BehavioralEntity::ProcessInterruption(int32_t interruption_id)
{
    logger.LogInfo("Processing interruption with id: " + std::to_string(interruption_id),
        "BehavioralEntity");
    // Check handler exists
    if (!interruption_handlers.contains(interruption_id))
    {
        logger.LogError("Could not find interruption handler with id: " + std::to_string(interruption_id),
            "ProcessInterruption");

        HandleSequenceFailure();
        return;
    }

    auto sequence = interruption_handlers.at(interruption_id);

    logger.LogInfo("Will process interruption " + std::to_string(interruption_id) + " with sequence " +
        std::to_string(sequence->GetSequenceId()) + " for entity: " + std::to_string(entity_id), "BehavioralEntity");

    // Context preservation
    if (sequences.top()->GetSequenceState() == SequenceState::WAITING_FOR_ACTION)
    {
        auto action = content_provider.GetActionById(current_action_id);
        if (!action)
        {
            logger.LogError("Could not find action with id: " + std::to_string(current_action_id),
                "ProcessInterruption");

            HandleSequenceFailure();
            return;
        }

        if (action->GetInterruptionBehavior() == InterruptionBehaviorType::RESUMABLE)
        {
            memory.UpdateInterruptionMemory(current_action_id, sequences.top()->GetSequenceId(), sequences.top()->GetCurrentNodeId(),
            current_action_target_id, time_manager.GetCurrentTime());
        }
        else
        {
            logger.LogInfo("The current action is not resumable so no context will be saved in the interruption memory for "
                           "entity " + std::to_string(entity_id), "ProcessInterruption");
        }
    }

    // Sequence State Management
    sequences.top()->SetSequenceState(SequenceState::INTERRUPTED);

    // Response sequence activation
    sequences.push(sequence);
}
