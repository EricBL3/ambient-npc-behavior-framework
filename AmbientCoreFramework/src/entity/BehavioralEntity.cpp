#include "BehavioralEntity.h"
#include <algorithm>
#include <tracy/Tracy.hpp>

#include "../behavior/sequence_nodes/ActionSequenceNode.h"
#include "../behavior/sequence_nodes/NestedSequenceNode.h"
#include "services/composition/ServiceBuilder.h"

using namespace AmbientCharacterBehavior;

bool BehavioralEntity::CanUpdate() const
{
    if (sequences.empty())
    {
        return !is_processing;
    }

    return !is_processing && sequences.top()->GetSequenceState() != SequenceState::WAITING_FOR_ACTION && !is_halted;
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
         "GetFallbackSequenceById");
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
            std::to_string(interruption_id) + " not found", "FindInterruptionHandler");

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
    ZoneScoped;
    ZoneText("entity_id", 9);
    ZoneValue(entity_id);
    ZoneText("sequence_id", 11);
    ZoneValue(sequences.top()->GetSequenceId());
    ZoneText("sequence_state", 14);
    ZoneValue(static_cast<uint64_t>(sequence_state));

    logger.LogInfo("Executing sequence state " + ToString(sequence_state) + " for entity: " +
        std::to_string(entity_id), "ExecuteSequenceStep");

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
                std::to_string(current_action_id), "ExecuteSequenceStep");
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
            "HandleEmptySequences");

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
    ZoneScoped;

    logger.LogInfo("Handling sequence startup for entity: " + std::to_string(entity_id),
        "HandleSequenceStartup");

    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
    sequences.top()->ResetCurrentNodeToEntry();
    fallback_attempt_count = 0;
    is_halted = false;
}

void BehavioralEntity::ProcessCurrentNode()
{
    ZoneScoped;

    logger.LogInfo("Processing current node for entity: " + std::to_string(entity_id),
        "ProcessCurrentNode");

    const auto current_node = TryGetCurrentNode("ProcessCurrentNode");
    if (!current_node)
    {
        MarkSequenceFailed();
        return;
    }

    ExecuteCurrentNode(current_node);
}

SequenceNode* BehavioralEntity::TryGetCurrentNode(const std::string& context)
{
    ZoneScoped;

    auto current_node = sequences.top()->FindCurrentNode();
    if (!current_node)
    {
        logger.LogError("Could not find the current node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()),
            context);
    }

    return current_node;
}

void BehavioralEntity::ExecuteCurrentNode(const SequenceNode* current_node)
{
    ZoneScoped;

    auto current_node_type = current_node->GetNodeType();
    if (current_node_type == SequenceNodeType::ACTION_NODE)
    {
        logger.LogInfo("Will execute action node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()) +
            " for entity with id: " + std::to_string(entity_id), "ExecuteCurrentNode");

        ExecuteActionNode(current_node);
    }
    else if (current_node_type == SequenceNodeType::NESTED_SEQUENCE_NODE)
    {
        logger.LogInfo("Will execute nested sequence node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()) +
            " for entity with id: " + std::to_string(entity_id), "ExecuteCurrentNode");

        ExecuteNestedSequenceNode(current_node);
    }
    else if (current_node_type == SequenceNodeType::END_SEQUENCE_NODE)
    {
        logger.LogInfo("Will execute end node with id: " + std::to_string(sequences.top()->GetCurrentNodeId()) +
            " for entity with id: " + std::to_string(entity_id), "ExecuteCurrentNode");

        ExecuteEndSequenceNode(current_node);
    }
    else
    {
        logger.LogWarning("The current node type is not supported", "ExecuteCurrentNode");
        sequences.top()->SetSequenceState(SequenceState::FAILED);
    }
}

void BehavioralEntity::ExecuteEndSequenceNode(const SequenceNode* current_node)
{
    ZoneScoped;

    logger.LogInfo("Reached end of sequence for entity " + std::to_string(entity_id),
                   "ExecuteEndSequenceNode");

    fallback_attempt_count = 0;
    sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
    sequences.pop();
}

void BehavioralEntity::ExecuteNestedSequenceNode(const SequenceNode* current_node)
{
    ZoneScoped;

    auto nested_sequence_node = dynamic_cast<const NestedSequenceNode*>(current_node);
    if (!nested_sequence_node)
    {
        logger.LogError("The current node type is not of type nested sequence node",
            "ExecuteNestedSequenceNode");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    sequences.top()->SetSequenceState(SequenceState::IN_SUBSEQUENCE);

    auto nested_sequence = TryGetSequence(nested_sequence_node->GetTargetSequenceId(), "ExecuteNestedSequenceNode");

    if (!nested_sequence)
    {
        MarkSequenceFailed();
        return;
    }

    auto nested_sequence_instance = nested_sequence->CreateInstance();
    sequences.emplace(nested_sequence_instance);
}

void BehavioralEntity::HandleSubsequenceCompletion()
{
    ZoneScoped;

    logger.LogInfo("Finished running subsequence for entity" + std::to_string(entity_id),
        "HandleSubsequenceCompletion");

    sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
    HandleNodeExecutionCompletion();
}

void BehavioralEntity::ExecuteActionNode(const SequenceNode* current_node)
{
    ZoneScoped;

    auto action = LookupActionFromCurrentNode(current_node);
    if (!action)
    {
        MarkSequenceFailed();
        return;
    }

    // Acquire target entity
    FrameworkEntity* target_entity = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = GetActionTargetEntity(action);

        if (!target_entity)
        {
            logger.LogWarning("No valid entities found for action " + std::to_string(action->GetActionId()) +
                " - triggering sequence failure","ExecuteActionNode");

            sequences.top()->SetSequenceState(SequenceState::FAILED);
            return;
        }
    }

    InitiateActionExecution(action, target_entity);
}

std::shared_ptr<Action> BehavioralEntity::LookupActionFromCurrentNode(const SequenceNode* current_node) const
{
    ZoneScoped;

    auto action_sequence_node = dynamic_cast<const ActionSequenceNode*>(current_node);
    if (!action_sequence_node)
    {
        logger.LogError("The current node type is not of type action sequence node",
            "ExecuteActionNode");

        return nullptr;
    }

    return TryGetAction(action_sequence_node->GetTargetActionId(),"ExecuteActionNode");
}

void BehavioralEntity::InitiateActionExecution(const std::shared_ptr<Action>& action, FrameworkEntity* target_entity, bool apply_immediate_effects)
{
    ZoneScoped;

    // Apply immediate effects
    if (apply_immediate_effects)
    {
        ApplyActionEffects(action->GetImmediateEffects(), target_entity);
    }

    // Start action
    void* target_entity_handle = nullptr;
    if (target_entity)
    {
        current_action_target_id = target_entity->GetEntityId();
        target_entity_handle = target_entity->GetEntityHandle();
    }

    current_action_id = action->GetActionId();
    current_action_token++;

    logger.LogInfo("Calling start character action for entity: " + std::to_string(entity_id) + " with action id: " +
        std::to_string(current_action_id) + " and token: " + std::to_string(current_action_token), "InitiateActionExecution");

    start_character_action_provider.StartCharacterAction(entity_handle, current_action_id, current_action_token,
        action->GetMaxDuration(), target_entity_handle);

    sequences.top()->SetSequenceState(SequenceState::WAITING_FOR_ACTION);
}

FrameworkEntity* BehavioralEntity::GetActionTargetEntity(const std::shared_ptr<Action>& action)
{
    ZoneScoped;

    FrameworkEntity* target_entity = nullptr;

    // Evaluate entities
    std::vector<FrameworkEntity*> entities = entity_query.GetEntitiesSupportingAction(action->GetActionId());

    // Filter to have only the entities that can be done (precondition satisfaction)
    std::vector<int32_t> entity_ids = GetValidEntityIds(entities, action->GetPreconditionsForTarget(StateOperationTarget::ENTITY));


    // Select best entity
    auto selected_entity_id = memory.GetLeastRecentlyUsedEntityIdForAction(action->GetActionId(), entity_ids);
    target_entity = entity_query.GetEntityFromId(selected_entity_id);

    return target_entity;
}

std::vector<int32_t> BehavioralEntity::GetValidEntityIds(const std::vector<FrameworkEntity*>& entities, const std::vector<StateOperation>* preconditions)
{
    std::vector<int32_t> entity_ids;

    for (auto entity : entities)
    {
        if (EvaluatePreconditions(preconditions, entity))
        {
            entity_ids.push_back(entity->GetEntityId());
        }
    }

    return entity_ids;
}

bool BehavioralEntity::EvaluatePreconditions(const std::vector<StateOperation>* preconditions, FrameworkEntity* other)
{
    if (!preconditions || preconditions->empty())
    {
        return true;
    }

    for (const auto& precondition : *preconditions)
    {
        FrameworkEntity* target_entity = nullptr;
        switch (precondition.GetTarget())
        {
            case StateOperationTarget::SELF:
                target_entity = this;
                break;
            case StateOperationTarget::ENTITY:
                target_entity = other;
                break;
            default:
                break;
        }

        if (!state_operation_evaluator.ProcessStateOperation(precondition, target_entity))
        {
            return false;
        }
    }
    return true;
}

void BehavioralEntity::ApplyActionEffects(const std::vector<StateOperation> & effects, FrameworkEntity* target_entity)
{
    ZoneScoped;

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
        " entity " + std::to_string(entity_id), "ApplyActionEffects");
}

void BehavioralEntity::CompleteAction(int32_t action_id, int64_t action_token)
{

    if (CompletedCurrentAction(action_id, action_token))
    {
        logger.LogInfo("entity with id: " + std::to_string(entity_id) + " has completed action with id: " +
            std::to_string(action_id) + " and token: " + std::to_string(action_token) ,"CompleteAction");

        ApplyCompletionEffects(action_id);
        memory.UpdateActionMemory(action_id, current_action_target_id, time_manager.GetCurrentTime());

        // Reset current_action_target_id and current_action_id to invalid value.
        current_action_target_id = -1;
        current_action_id = -1;
        fallback_attempt_count = 0;

        if (!sequences.empty())
        {
            sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
            logger.LogInfo("entity with id: " + std::to_string(entity_id) + " is now in NODE_EXECUTED step",
                "CompleteAction");
        }
        else
        {
            HandleEmptySequences();
        }

        is_processing = false;
    }
}

void BehavioralEntity::ApplyCompletionEffects(int32_t action_id)
{
    auto action = TryGetAction(action_id, "CompleteAction");
    if (!action)
    {
        MarkSequenceFailed();
        return;
    }

    FrameworkEntity* target_entity = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = TryGetEntity(current_action_target_id, "CompleteAction");
        if (!target_entity)
        {
            MarkSequenceFailedAndStopProcessing();
            return;
        }
    }

    ApplyActionEffects(action->GetCompletionEffects(), target_entity);
}

bool BehavioralEntity::CompletedCurrentAction(int32_t action_id, int64_t action_token) const
{
    if (action_token != current_action_token || action_id != current_action_id)
    {
        logger.LogWarning("The completed action with id: " + std::to_string(action_id) + " and token: " +
            std::to_string(action_token) + " is not the same as the currently executing action for entity with id: " +
            std::to_string(entity_id),"CompletedCurrentAction");

        return false;
    }

    return true;
}

void BehavioralEntity::HandleNodeExecutionCompletion()
{
    ZoneScoped;

    auto current_node = TryGetCurrentNode("HandleNodeExecutionCompletion");
    if (!current_node)
    {
        MarkSequenceFailed();
        return;
    }

    current_node->MarkAsCompleted();

    // Evaluate transitions
    std::vector<Transition> transitions = sequences.top()->GetValidTransitionsFromCurrentNode();

    // Filter to have only the nodes that can be transitioned to (precondition satisfaction)
    std::vector<int32_t> node_ids = GetValidTransitionNodeIds(transitions);

    // Select best node to transition to
    auto selected_node_id = memory.GetLeastRecentlyVisitedNodeId(node_ids);
    if (!sequences.top()->TrySetCurrentNode(selected_node_id))
    {
        logger.LogError("Could not set current node with id: " + std::to_string(selected_node_id),
            "HandleNodeExecutionCompletion");

        sequences.top()->SetSequenceState(SequenceState::FAILED);
        return;
    }

    memory.UpdateTransitionMemory(selected_node_id, time_manager.GetCurrentTime());
    sequences.top()->FindCurrentNode()->ResetCompletion();
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
    fallback_attempt_count = 0;
}

std::vector<int32_t> BehavioralEntity::GetValidTransitionNodeIds(const std::vector<Transition> &transitions)
{
    std::vector<int32_t> node_ids;

    for (const auto& transition : transitions)
    {
        if (EvaluatePreconditions(transition.GetPreconditionsForTarget(StateOperationTarget::SELF), nullptr) &&
            EvaluatePreconditions(transition.GetPreconditionsForTarget(StateOperationTarget::ENVIRONMENT), nullptr))
        {
            node_ids.push_back(transition.GetDestinationNodeId());
        }
    }

    return node_ids;
}

void BehavioralEntity::HandleSequenceFailure()
{
    ZoneScoped;

    fallback_attempt_count++;
    if (fallback_attempt_count >= MAX_FALLBACK_ATTEMPTS)
    {
        logger.LogError("Entity " + std::to_string(entity_id) + " exceeded max fallback attempts, halting",
            "HandleSequenceFailure");

        // is halted is turned to true to avoid updating this character
        is_halted = true;
        return;
    }

    logger.LogInfo("Handling sequence failure for entity: " + std::to_string(entity_id),
        "HandleSequenceFailure");

    memory.ClearSequenceInterruptionMemories(sequences.top()->GetSequenceId());
    sequences.top()->ResetCurrentNodeToEntry();
    sequences.pop();

    if (fallback_sequences.empty())
    {
        logger.LogError("No fallback sequences available for entity " +
                       std::to_string(entity_id), "HandleSequenceFailure");

        return;
    }

    auto fallback_sequence_template = fallback_sequences[rand() % fallback_sequences.size()];
    auto fallback_instance = fallback_sequence_template->CreateInstance();

    logger.LogInfo("Entity with id: " + std::to_string(entity_id) + " will now follow fallback sequence with id: " +
        std::to_string(fallback_instance->GetSequenceId()), "HandleSequenceFailure");

    sequences.push(fallback_instance);
}

void BehavioralEntity::HandleInterruptionRecovery()
{
    ZoneScoped;

    logger.LogInfo("Handling interruption recovery for entity: " + std::to_string(entity_id),
        "HandleInterruptionRecovery");
    // Check if sequence was executing action
    current_action_id = RecoverCurrentActionId();
    if (current_action_id >= 0)
    {
        logger.LogInfo("Checking if action " + std::to_string(current_action_id) + " is resumable for entity " +
            std::to_string(entity_id), "HandleInterruptionRecovery");

        auto action = TryGetAction(current_action_id, "HandleInterruptionRecovery");

        if (!action)
        {
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

int32_t BehavioralEntity::RecoverCurrentActionId()
{
    const auto current_node = TryGetCurrentNode("HandleInterruptionRecovery");
    if (!current_node)
    {
        logger.LogError("Cannot recover action ID - no current node",
            "RecoverCurrentActionId");

        return -1;
    }

    const auto action_node = dynamic_cast<ActionSequenceNode*>(current_node);
    if (!action_node)
    {
        logger.LogWarning("Current node is not an ActionSequenceNode (node_id: " +
            std::to_string(current_node->GetNodeId()) + "), cannot recover action ID",
            "RecoverCurrentActionId");

        return -1;
    }

    const auto recovered_action_id = action_node->GetTargetActionId();
    logger.LogInfo("Recovered action ID " + std::to_string(recovered_action_id) + " from node " +
        std::to_string(current_node->GetNodeId()),"RecoverCurrentActionId");

    return recovered_action_id;
}

void BehavioralEntity::AttemptActionResumption()
{

    logger.LogInfo("Attempting to resume action " + std::to_string(current_action_id) + " from interruption for entity" +
        std::to_string(entity_id), "AttemptActionResumption");

    //todo: finding interruption memories could be simplified if I reconsider what is actually necessary when finding them.
    // This could then be used to recover the memory earlier instead of relying on finding the current_action_id of the interrupted sequence.
    auto interruption_memory = memory.FindInterruptionMemory(current_action_id, sequences.top()->GetSequenceId(),
                sequences.top()->GetCurrentNodeId());

    if (!interruption_memory)
    {
        logger.LogInfo("No interruption memory exists for action " + std::to_string(current_action_id) +
            " for entity " + std::to_string(entity_id), "AttemptActionResumption");

        sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
        return;
    }

    auto action = TryGetAction(interruption_memory->GetInterruptedActionId(),"AttemptActionResumption");
    if (!action)
    {
        memory.RemoveInterruptionMemory(interruption_memory);
        MarkSequenceFailed();
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

    auto target_entity = TryGetEntity(target_entity_id, "ValidateResumptionContext");
    if (!target_entity)
    {
        return false;
    }

    if (!target_entity->SupportsAction(action->GetActionId()))
    {
        logger.LogInfo("Target entity " + std::to_string(target_entity_id) +
            " no longer supports action " + std::to_string(action->GetActionId()),
            "ValidateResumptionContext");
        return false;
    }

    if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::ENTITY), target_entity))
    {
        logger.LogInfo("Precondition no longer satisfied for action " +
                std::to_string(action->GetActionId()), "ValidateResumptionContext");
        return false;
    }

    return true;
}

void BehavioralEntity::ResumeActionWithSavedContext(const std::shared_ptr<Action>& action, const InterruptionMemory* interruption_memory)
{
    FrameworkEntity* target_entity = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = TryGetEntity(interruption_memory->GetInterruptedTargetEntityId(), "ResumeActionWithSavedContext");
        if (!target_entity)
        {
            MarkSequenceFailed();
            return;
        }
    }

    InitiateActionExecution(action, target_entity, false);
}

void BehavioralEntity::ProcessInterruption(int32_t interruption_id)
{
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
        std::to_string(sequence->GetSequenceId()) + " for entity: " + std::to_string(entity_id),
        "ProcessInterruption");

    // Context preservation
    if (sequences.top()->GetSequenceState() == SequenceState::WAITING_FOR_ACTION)
    {
        auto action = TryGetAction(current_action_id, "ProcessInterruption");
        if (!action)
        {
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

std::shared_ptr<Action> BehavioralEntity::TryGetAction(int32_t action_id, const std::string &context) const
{
    auto action = content_provider.GetActionById(action_id);
    if (!action)
    {
        logger.LogError(
            "Could not find action with id: " + std::to_string(action_id),
            context);
    }
    return action;
}

FrameworkEntity* BehavioralEntity::TryGetEntity(int32_t entity_id, const std::string& context) const
{
    auto entity = entity_query.GetEntityFromId(entity_id);
    if (!entity)
    {
        logger.LogError(
            "Could not find entity with id: " + std::to_string(entity_id),
            context);
    }
    return entity;
}

std::shared_ptr<Sequence> BehavioralEntity::TryGetSequence(int32_t sequence_id, const std::string& context) const
{
    auto sequence = content_provider.GetSequenceById(sequence_id);
    if (!sequence)
    {
        logger.LogError(
            "Could not find sequence with id: " + std::to_string(sequence_id),
            context);
    }
    return sequence;
}

void BehavioralEntity::MarkSequenceFailed()
{
    sequences.top()->SetSequenceState(SequenceState::FAILED);
}

void BehavioralEntity::MarkSequenceFailedAndStopProcessing()
{
    sequences.top()->SetSequenceState(SequenceState::FAILED);
    is_processing = false;
}