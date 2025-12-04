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

    const auto current_node = sequences.top()->FindCurrentNode();
    if (!current_node)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::NODE_NOT_FOUND,
            .node_id = sequences.top()->GetCurrentNodeId(),
            .additional_info = "ProcessCurrentNode"
        });

        return;
    }

    ExecuteCurrentNode(current_node);
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
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::INVALID_NODE_TYPE,
            .additional_info = "The node type is not supported. ExecuteCurrentNode"
        });
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
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::INVALID_NODE_TYPE,
            .additional_info = "ExecuteNestedSequenceNode"
        });

        return;
    }

    sequences.top()->SetSequenceState(SequenceState::IN_SUBSEQUENCE);

    auto nested_sequence = content_provider.GetSequenceById(nested_sequence_node->GetTargetSequenceId());
    if (!nested_sequence)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::SEQUENCE_NOT_FOUND,
            .sequence_id = nested_sequence_node->GetTargetSequenceId(),
            .additional_info = "ExecuteNestedSequenceNode"
        });

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

    // 1. Find action
    auto action = LookupActionFromCurrentNode(current_node);
    if (!action)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::ACTION_NOT_FOUND,
            .additional_info = "ExecuteActionNode"
        });
        return;
    }

    // 2. Check SELF and ENVIRONMENT preconditions
    auto validation = ValidateActionPreconditions(action);
    if (!validation.Passed())
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::PRECONDITIONS_FAILED,
            .action_id = action->GetActionId(),
            .additional_info = validation.failed_target.has_value() ?
                "Failed " + ToString(validation.failed_target.value()) + " preconditions" : ""
        });

        return;
    }

    // 3. Acquire target entity
    FrameworkEntity* target_entity = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = GetActionTargetEntity(action);

        if (!target_entity)
        {
            HandleRuntimeFailure({
                .reason = RuntimeFailureReason::NO_VALID_ENTITIES,
                .action_id = action->GetActionId(),
                .additional_info = "ExecuteActionNode"
            });

            return;
        }
    }

    // 4. Execute action
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

    return content_provider.GetActionById(action_sequence_node->GetTargetActionId());
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
    else
    {
        current_action_target_id = -1;
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

    std::vector<FrameworkEntity*> entities = entity_query.GetEntitiesSupportingAction(action->GetActionId());
    auto preconditions = action->GetPreconditionsForTarget(StateOperationTarget::ENTITY);

    std::vector<int32_t> unused_entities;
    unused_entities.reserve(entities.size());
    std::vector<int32_t> previously_used_entities;
    previously_used_entities.reserve(entities.size());

    // Separate unused from used entities
    for (auto* entity : entities)
    {
        // skip current entity if we checked it above
        if (entity->GetEntityId() == current_action_target_id)
        {
            continue;
        }

        if (!EvaluatePreconditions(preconditions, entity))
        {
            // skip invalid entities
            continue;
        }

        // Check if entity has been used for this action before
        if (!memory.HasActionMemory(action->GetActionId(), entity->GetEntityId()))
        {
            unused_entities.push_back(entity->GetEntityId());
        }
        else
        {
            previously_used_entities.push_back(entity->GetEntityId());
        }
    }

    // Exploration by choosing randomly an unused entity
    if (!unused_entities.empty())
    {
        auto random_index = GetRandomIndex(unused_entities.size());

        return entity_query.GetEntityFromId(unused_entities[random_index]);
    }

    // Exploitation by using least recently used among used entities
    if (previously_used_entities.empty())
    {
        return nullptr;
    }

    auto lru_options = memory.GetLeastRecentlyUsedEntityIdsForAction(action->GetActionId(), previously_used_entities);
    if (lru_options.empty())
    {
        return nullptr;
    }

    auto random_index = GetRandomIndex(lru_options.size());
    auto selected_entity_id = lru_options[random_index];

    return entity_query.GetEntityFromId(selected_entity_id);
}

BehavioralEntity::PreconditionValidation BehavioralEntity::ValidateActionPreconditions(
    const std::shared_ptr<Action> &action, FrameworkEntity *target_entity)
{
    ZoneScoped;

    if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::SELF), nullptr))
    {
        return { false, StateOperationTarget::SELF};
    }

    if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::ENVIRONMENT), nullptr))
    {
        return { false, StateOperationTarget::ENVIRONMENT};
    }

    if (target_entity)
    {
        if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::ENTITY), target_entity))
        {
            return { false, StateOperationTarget::ENTITY};
        }
    }

    return { true, std::nullopt };
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

        // Reset and current_action_id to invalid value.
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
    auto action = content_provider.GetActionById(action_id);
    if (!action)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::ACTION_NOT_FOUND,
            .action_id = action_id,
            .additional_info = "ApplyCompletionEffects"
        });

        return;
    }

    FrameworkEntity* target_entity = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = entity_query.GetEntityFromId(current_action_target_id);
        if (!target_entity)
        {
            HandleRuntimeFailure({
                .reason = RuntimeFailureReason::ENTITY_NOT_FOUND,
                .action_id = action_id,
                .entity_id = current_action_target_id,
                .additional_info = "ApplyCompletionEffects",
                .should_stop_processing = true
            });

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

    auto current_node = sequences.top()->FindCurrentNode();
    if (!current_node)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::NODE_NOT_FOUND,
            .node_id = sequences.top()->GetCurrentNodeId(),
            .additional_info = "HandleNodeExecutionCompletion",
        });

        return;
    }

    auto selected_node_id = GetNodeIdForNextTransition();
    if (!selected_node_id.has_value())
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::NO_VALID_TRANSITIONS,
            .node_id = sequences.top()->GetCurrentNodeId(),
            .additional_info = "HandleNodeExecutionCompletion",
        });

        return;
    }

    if (!sequences.top()->TrySetCurrentNode(selected_node_id.value()))
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::NODE_NOT_FOUND,
            .node_id = selected_node_id.value(),
            .additional_info = "HandleNodeExecutionCompletion",
        });

        return;
    }

    current_node->MarkAsCompleted();
    memory.UpdateTransitionMemory(selected_node_id.value(), time_manager.GetCurrentTime());
    sequences.top()->FindCurrentNode()->ResetCompletion();
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
    fallback_attempt_count = 0;
}

std::optional<int32_t> BehavioralEntity::GetNodeIdForNextTransition()
{
    ZoneScoped;

    std::vector<Transition> transitions = sequences.top()->GetValidTransitionsFromCurrentNode();

    std::vector<int32_t> unused_transitions;
    unused_transitions.reserve(transitions.size());
    std::vector<int32_t> previously_used_transitions;
    previously_used_transitions.reserve(transitions.size());

    // Separate unused from used transitions
    for (const auto& transition : transitions)
    {
        if (!EvaluatePreconditions(transition.GetPreconditionsForTarget(StateOperationTarget::SELF), nullptr) ||
            !EvaluatePreconditions(transition.GetPreconditionsForTarget(StateOperationTarget::ENVIRONMENT), nullptr))
        {
            // skip invalid transitions
            continue;
        }

        // Check if the transition has been used before
        if (!memory.HasTransitionMemory(transition.GetDestinationNodeId()))
        {
            unused_transitions.push_back(transition.GetDestinationNodeId());
        }
        else
        {
            previously_used_transitions.push_back(transition.GetDestinationNodeId());
        }
    }

    // Exploration phase choosing randomly between unused transitions
    if (!unused_transitions.empty())
    {
        auto random_index = GetRandomIndex(unused_transitions.size());

        return unused_transitions[random_index];
    }

    // Exploitation phase using least recently used
    if (previously_used_transitions.empty())
    {
        return std::nullopt;
    }

    auto lru_options = memory.GetLeastRecentlyVisitedNodeIds(previously_used_transitions);
    if (lru_options.empty())
    {
        return std::nullopt;
    }

    auto random_index = GetRandomIndex(lru_options.size());
    return lru_options[random_index];
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

        auto action = content_provider.GetActionById(current_action_id);
        if (!action)
        {
            HandleRuntimeFailure({
                .reason = RuntimeFailureReason::ACTION_NOT_FOUND,
                .action_id = current_action_id,
                .additional_info = "HandleInterruptionRecovery",
            });
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
    const auto current_node = sequences.top()->FindCurrentNode();
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

    auto action = content_provider.GetActionById(interruption_memory->GetInterruptedActionId());
    if (!action)
    {
        memory.RemoveInterruptionMemory(interruption_memory);
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::ACTION_NOT_FOUND,
            .action_id = interruption_memory->GetInterruptedActionId(),
            .additional_info = "AttemptActionResumption"
        });

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
        target_entity = entity_query.GetEntityFromId(interruption_memory->GetInterruptedTargetEntityId());
        if (!target_entity)
        {
            HandleRuntimeFailure({
                .reason = RuntimeFailureReason::ENTITY_NOT_FOUND,
                .entity_id = interruption_memory->GetInterruptedTargetEntityId(),
                .additional_info = "ResumeActionWithSavedContext"
            });

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
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::INTERRUPTION_NOT_FOUND,
            .interruption_id = interruption_id,
            .additional_info = "ProcessInterruption"
        });

        return;
    }

    auto sequence = interruption_handlers.at(interruption_id);

    logger.LogInfo("Will process interruption " + std::to_string(interruption_id) + " with sequence " +
        std::to_string(sequence->GetSequenceId()) + " for entity: " + std::to_string(entity_id),
        "ProcessInterruption");

    // Context preservation
    if (sequences.top()->GetSequenceState() == SequenceState::WAITING_FOR_ACTION)
    {
        auto action = content_provider.GetActionById(current_action_id);
        if (!action)
        {
            HandleRuntimeFailure({
                .reason = RuntimeFailureReason::ACTION_NOT_FOUND,
                .action_id = current_action_id,
                .additional_info = "ProcessInterruption"
            });

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
    sequences.push(sequence->CreateInstance());
}

void BehavioralEntity::HandleRuntimeFailure(const RuntimeFailureContext &context)
{
    std::string reason_str;
    switch (context.reason)
    {
        case RuntimeFailureReason::NODE_NOT_FOUND:
            reason_str = "Node not found " + (context.node_id > -1 ? " (ID: " + std::to_string(context.node_id) + ")"
                : "");

            break;
        case RuntimeFailureReason::ACTION_NOT_FOUND:
            reason_str = "Action not found " + (context.action_id > -1 ? " (ID: " + std::to_string(context.action_id) + ")"
                : "");
            break;
        case RuntimeFailureReason::ENTITY_NOT_FOUND:
            reason_str = "Entity not found " + (context.entity_id > -1 ? " (ID: " + std::to_string(context.entity_id) + ")"
                : "");
            break;
        case RuntimeFailureReason::SEQUENCE_NOT_FOUND:
            reason_str = "Sequence not found " + (context.sequence_id > -1 ? " (ID: " + std::to_string(context.sequence_id) + ")"
                : "");
            break;
        case RuntimeFailureReason::NO_VALID_ENTITIES:
            reason_str = "No valid entities for action (ID: " + std::to_string(context.action_id) + ")";
            break;
        case RuntimeFailureReason::NO_VALID_TRANSITIONS:
            reason_str = "No valid transitions found from node " + std::to_string(context.node_id);
            break;
        case RuntimeFailureReason::PRECONDITIONS_FAILED:
            reason_str = "Preconditions failed";
            break;
        case RuntimeFailureReason::INVALID_NODE_TYPE:
            reason_str = "Invalid node type";
            break;
        case RuntimeFailureReason::INTERRUPTION_NOT_FOUND:
            reason_str = "Interruption not found " + (context.interruption_id > -1 ? " (ID: " + std::to_string(context.interruption_id) + ")"
                : "");
            break;
    }

    logger.LogError("Action execution failed for entity " + std::to_string(entity_id) + ": " + reason_str +
        (context.additional_info.empty() ? "" : " - " + context.additional_info), "HandleRuntimeFailure");

    sequences.top()->SetSequenceState(SequenceState::FAILED);

    // Stop processing if context indicates we should
    if (context.should_stop_processing) {
        is_processing = false;
    }
}

int32_t BehavioralEntity::GetRandomIndex(int32_t max_exclusive)
{
    if (max_exclusive <= 1 )
    {
        return 0;
    }

    std::uniform_int_distribution<int32_t> dist(0, max_exclusive - 1);
    return dist(rng);
}
