#include "BehavioralEntity.h"
#include <algorithm>
#include <tracy/Tracy.hpp>

#include "EntityMetricInfo.h"
#include "../behavior/sequence_nodes/ActionSequenceNode.h"
#include "../behavior/sequence_nodes/NestedSequenceNode.h"
#include "services/composition/ServiceBuilder.h"

using namespace AmbientCharacterBehavior;

// ═══════════════════════════════════════════════════════════════════════════
// CONSTRUCTION & CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

BehavioralEntity::BehavioralEntity(
    ContentRegistryServices &services,
    void *entity_handle,
    int32_t entity_id,
    int32_t max_transition_memories,
    int32_t max_action_memories,
    int32_t max_interruption_memories,
    std::string name
    ) : FrameworkEntity(entity_handle, entity_id, std::move(name)),
        services(services),
        memory(max_transition_memories, max_action_memories,
            max_interruption_memories,
            services.behavioral_evaluation.simulation_state.data_access.foundation),
        main_sequence(nullptr),
        current_action_target_id(-1),
        is_processing(false),
        current_action_token(0),
        current_action_id(-1),
        fallback_attempt_count(0),
        is_halted(false) {}

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

void BehavioralEntity::AddInterruptionHandler(int32_t interruption_id, const std::shared_ptr<Sequence> &handler)
{
    if (handler)
    {
        interruption_handlers.insert_or_assign(interruption_id, handler->CreateInstance());
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN EXECUTION LOOP
// ═══════════════════════════════════════════════════════════════════════════

void BehavioralEntity::ExecuteCurrentSequence()
{
    // Process pending interruptions before sequence execution to ensure
    // time-critical responses (e.g., danger reactions) aren't delayed.
    if (!pending_interruptions.empty())
    {
        ProcessPendingInterruptions();
        return;
    }

    // Set processing flag to prevent re-entrant calls during this update.
    // This flag is checked by CanUpdate() and CompleteAction().
    is_processing = true;

    if (sequences.empty())
    {
        // Pushes main sequence or halts if null
        HandleEmptySequences();
        return;
    }

    ExecuteSequenceStep(sequences.top()->GetSequenceState());

    // Allow next update
    is_processing = false;
}

bool BehavioralEntity::CanUpdate() const
{
    if (sequences.empty())
    {
        return !is_processing;
    }

    return !is_processing && sequences.top()->GetSequenceState() != SequenceState::WAITING_FOR_ACTION && !is_halted;
}

void BehavioralEntity::ExecuteSequenceStep(SequenceState sequence_state)
{
    ZoneScoped;

    auto currentSequenceId = sequences.top()->GetSequenceId();

    ZoneText("entity_id", 9);
    ZoneValue(entity_id);
    ZoneText("sequence_id", 11);
    ZoneValue(currentSequenceId);
    ZoneText("sequence_state", 14);
    ZoneValue(static_cast<uint64_t>(sequence_state));

    Logger().LogInfo("Executing sequence state " + ToString(sequence_state) + " for entity: " +
        std::to_string(entity_id) + " in sequence: " + std::to_string(currentSequenceId),
        "ExecuteSequenceStep");

    // Routes execution to specialized handlers based on current sequence state.
    // Each handler is responsible for state transition logic.
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
            // No operation: waiting for external CompleteAction() callback
            Logger().LogInfo("Waiting for character '" + std::to_string(entity_id) +
                "' to complete action with id: " + std::to_string(current_action_id),
                "ExecuteSequenceStep");
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
        Logger().LogError("character with id: " + std::to_string(entity_id) + " does not have a valid main "
            "sequence.", "HandleEmptySequences");

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

    Logger().LogInfo("Handling sequence (" + std::to_string(sequences.top()->GetSequenceId()) + ") startup "
        "for entity: " + std::to_string(entity_id), "HandleSequenceStartup");

    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
    sequences.top()->ResetCurrentNodeToEntry();
    fallback_attempt_count = 0;
    is_halted = false;
}

// ═══════════════════════════════════════════════════════════════════════════
// NODE PROCESSING (Sequence Step Execution)
// ═══════════════════════════════════════════════════════════════════════════

void BehavioralEntity::ProcessCurrentNode()
{
    ZoneScoped;

    auto currentNodeId = sequences.top()->GetCurrentNodeId();
    Logger().LogInfo("Processing current node (" + std::to_string(currentNodeId) + ") in sequence (" +
        std::to_string(sequences.top()->GetSequenceId()) + ") for entity: " + std::to_string(entity_id),
        "ProcessCurrentNode");

    const auto current_node = sequences.top()->FindCurrentNode();
    if (!current_node)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::NODE_NOT_FOUND,
            .node_id = currentNodeId,
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
        Logger().LogWarning("The current node type is not supported", "ExecuteCurrentNode");
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::INVALID_NODE_TYPE,
            .additional_info = "The node type is not supported. ExecuteCurrentNode"
        });
    }
}

void BehavioralEntity::ExecuteActionNode(const SequenceNode* current_node)
{
    ZoneScoped;

    // STEP 1: LOOKUP ACTION
    auto action = LookupActionFromCurrentNode(current_node);
    if (!action)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::ACTION_NOT_FOUND,
            .additional_info = "ExecuteActionNode"
        });
        return;
    }

    Logger().LogInfo("Will execute action node with id: " +
            std::to_string(sequences.top()->GetCurrentNodeId()) + " and target action id: " +
            std::to_string(action->GetActionId()) + " for entity with id: " +
            std::to_string(entity_id), "ExecuteCurrentNode");

    // STEP 2: VALIDATE CONTEXT-FREE PRECONDITIONS
    // Check SELF and ENVIRONMENT preconditions (don't require entity)
    // This early validation avoids unnecessary entity selection work
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

    // STEP 3: ACQUIRE TARGET ENTITY (if required)
    // Uses memory-driven selection among precondition-valid entities
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

    // STEP 4: INITIATE EXECUTION
    InitiateActionExecution(action, target_entity);
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

    Logger().LogInfo("Will execute nested sequence node with id: " +
            std::to_string(sequences.top()->GetCurrentNodeId()) + " and target sequence id: " +
            std::to_string(nested_sequence_node->GetTargetSequenceId()) + " for entity with id: " +
            std::to_string(entity_id), "ExecuteCurrentNode");

    sequences.top()->SetSequenceState(SequenceState::IN_SUBSEQUENCE);

    auto nested_sequence = ContentProvider().GetSequenceById(
        nested_sequence_node->GetTargetSequenceId());

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

void BehavioralEntity::ExecuteEndSequenceNode(const SequenceNode* current_node)
{
    ZoneScoped;

    Logger().LogInfo("Will execute end node with id: " +
            std::to_string(sequences.top()->GetCurrentNodeId()) + " for entity with id: " +
            std::to_string(entity_id), "ExecuteCurrentNode");

    Logger().LogInfo("Reached end of sequence (" + std::to_string(sequences.top()->GetSequenceId()) +
        ") for entity " + std::to_string(entity_id), "ExecuteEndSequenceNode");

    fallback_attempt_count = 0;
    sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
    sequences.pop();
}

// ═══════════════════════════════════════════════════════════════════════════
// ACTION EXECUTION LIFECYCLE
// ═══════════════════════════════════════════════════════════════════════════

std::shared_ptr<Action> BehavioralEntity::LookupActionFromCurrentNode(const SequenceNode* current_node) const
{
    ZoneScoped;

    auto action_sequence_node = dynamic_cast<const ActionSequenceNode*>(current_node);
    if (!action_sequence_node)
    {
        Logger().LogError("The current node type is not of type action sequence node",
            "ExecuteActionNode");

        return nullptr;
    }

    return ContentProvider().GetActionById(action_sequence_node->GetTargetActionId());
}

void BehavioralEntity::InitiateActionExecution(const std::shared_ptr<Action>& action, FrameworkEntity* target_entity)
{
    ZoneScoped;

    // STEP 1: APPLY IMMEDIATE EFFECTS
    ApplyActionEffects(action->GetImmediateEffects(), target_entity);

    // STEP 2: RECORD EXECUTION CONTEXT
    // Store target entity for completion effects and resumption
    void* target_entity_handle = nullptr;
    if (target_entity)
    {
        current_action_target_id = target_entity->GetEntityId();
        target_entity_handle = target_entity->GetEntityHandle();
    }
    else
    {
        // Self-targeted action
        current_action_target_id = -1;
    }

    current_action_id = action->GetActionId();
    // Invalidates any stale callbacks
    current_action_token++;

    // STEP 3: RECORD IN MEMORY
    // Track this action-entity pair for future selection diversity
    memory.CreateActionMemory(current_action_id, current_action_target_id, TimeManager().GetCurrentTime());

    Logger().LogInfo("Calling start character action for entity: " + std::to_string(entity_id) + " with action "
        "id: " + std::to_string(current_action_id) + " and token: " + std::to_string(current_action_token),
        "InitiateActionExecution");

    // Log action event as:
    // event, timestamp, npc_id, event_type, action_id, target_entity_id, action_token
    nlohmann::json action_event = {
        {"event", "action"},
        { "ts", TimeManager().GetCurrentTime() },
        { "npc_id", entity_id},
        {"event_type", "start"},
        {"action_id", current_action_id},
        { "target_entity_id", current_action_target_id},
        { "action_token", current_action_token}
    };

    Logger().LogMetric(action_event);

    // STEP 4: DELEGATE TO GAME ENGINE
    // The game engine handles animation, pathfinding, audio, etc.
    // Will call back to CompleteAction() when done (or timeout triggers)
    ActionProvider().StartCharacterAction(entity_handle, current_action_id, current_action_token,
        action->GetActionDuration(), target_entity_handle);

    // STEP 5: TRANSITION STATE
    // Pause sequence execution until action completes
    sequences.top()->SetSequenceState(SequenceState::WAITING_FOR_ACTION);

    // STEP 6: REGISTER TIMEOUT
    // Fallback completion if engine never calls back
    ActionTimeoutManager().RegisterActionTimeout(entity_handle, current_action_id, current_action_token,
        TimeManager().GetCurrentTime(), action->GetActionTimeout());
}

void BehavioralEntity::ApplyActionEffects(const std::vector<StateOperation> & effects, FrameworkEntity* target_entity)
{
    ZoneScoped;

    for (const auto& effect: effects)
    {
        StateOperationContext context(this, target_entity);
        StateEvaluator().ProcessStateOperation(effect, context);

        // Log state change event as:
        // event, timestamp, npc_id, state_name, new_value, action_id
        auto state_name = SchemaManager().GetStateName(effect.GetStateKey());
        nlohmann::json event = {
            {"event", "state_change"},
            { "ts", TimeManager().GetCurrentTime() },
            { "npc_id", entity_id},
            {"state_name", state_name},
            {"new_value", effect.GetValue()},
            { "action_id", current_action_id}
        };

        Logger().LogMetric(event);
    }

    Logger().LogInfo("Applied " + std::to_string(effects.size()) + " effects for action (" +
        std::to_string(current_action_id) + ") processing of entity " + std::to_string(entity_id),
        "ApplyActionEffects");
}

void BehavioralEntity::CompleteAction(int32_t action_id, int64_t action_token)
{

    if (CompletedCurrentAction(action_id, action_token))
    {
        Logger().LogInfo("entity with id: " + std::to_string(entity_id) + " has completed action with id: " +
            std::to_string(action_id) + " and token: " + std::to_string(action_token) ,"CompleteAction");

        // Log action event as:
        // event, timestamp, npc_id, event_type, action_id, target_entity_id, action_token
        nlohmann::json action_event = {
            {"event", "action"},
            { "ts", TimeManager().GetCurrentTime() },
            { "npc_id", entity_id},
            {"event_type", "complete"},
            {"action_id", current_action_id},
            { "target_entity_id", current_action_target_id},
            { "action_token", current_action_token}
        };

        Logger().LogMetric(action_event);

        ActionTimeoutManager().UnregisterActionTimeout(entity_handle);
        ApplyCompletionEffects(action_id);

        // Reset and current_action_id to invalid value.
        current_action_id = -1;
        fallback_attempt_count = 0;

        if (!sequences.empty())
        {
            sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
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
    auto action = ContentProvider().GetActionById(action_id);
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
        target_entity = EntityQuery().GetEntityFromId(current_action_target_id);
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
        Logger().LogWarning("The completed action with id: " + std::to_string(action_id) + " and token: " +
            std::to_string(action_token) + " is not the same as the currently executing action for entity with id: " +
            std::to_string(entity_id),"CompletedCurrentAction");

        return false;
    }

    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// MEMORY-DRIVEN SELECTION
// ═══════════════════════════════════════════════════════════════════════════

FrameworkEntity* BehavioralEntity::GetActionTargetEntity(const std::shared_ptr<Action>& action)
{
    ZoneScoped;

    // PHASE 1: CANDIDATE COLLECTION
    // Query all entities that support this action type
    std::vector<FrameworkEntity*> entities = EntityQuery().GetEntitiesSupportingAction(action->GetActionId());
    const auto entity_preconditions =
        action->GetPreconditionsForTarget(StateOperationTarget::ENTITY);

    const auto entity_distance_preconditions =
        action->GetPreconditionsForTarget(StateOperationTarget::DISTANCE_TO_ENTITY);

    std::vector<int32_t> valid_entity_ids;
    valid_entity_ids.reserve(entities.size());

    // PHASE 2: PRECONDITION FILTERING
    // Retain only entities satisfying both ENTITY and DISTANCE preconditions
    for (auto* entity : entities)
    {
        Logger().LogInfo("Evaluating preconditions for entity: " + std::to_string(entity->GetEntityId()),
            "GetActionTargetEntity");

        // Validate entity state preconditions
        if (!EvaluatePreconditions(entity_preconditions, entity))
        {
            // skip invalid entities
            continue;
        }

        // Validate spatial preconditions (distance checks)
        if (!EvaluatePreconditions(entity_distance_preconditions, entity))
        {
            // skip invalid entities
            continue;
        }

        Logger().LogInfo("Preconditions for entity: " + std::to_string(entity_id) + " for action: " +
            std::to_string(action->GetActionId()) + " are valid.","GetActionTargetEntity");

        valid_entity_ids.push_back(entity->GetEntityId());
    }

    // PHASE 3: MEMORY-BASED SELECTION

    auto metric_info = new EntityMetricInfo{entity_id, name};
    auto selected_entity_id = memory.SelectActionEntityId(action->GetActionId(), valid_entity_ids,
        *metric_info);

    return selected_entity_id ? EntityQuery().GetEntityFromId(selected_entity_id.value()) : nullptr;
}

std::optional<int32_t> BehavioralEntity::GetNodeIdForNextTransition()
{
    ZoneScoped;

    std::vector<Transition> transitions = sequences.top()->GetValidTransitionsFromCurrentNode();

    std::vector<int32_t> valid_node_ids;
    valid_node_ids.reserve(transitions.size());

    // Obtain valid node_ids
    for (const auto& transition : transitions)
    {
        // skip invalid transitions
        if (!EvaluatePreconditions(transition.GetPreconditionsForTarget(StateOperationTarget::SELF), nullptr))
        {
            continue;
        }

        if (!EvaluatePreconditions(transition.GetPreconditionsForTarget(StateOperationTarget::ENVIRONMENT), nullptr))
        {
            continue;
        }

        valid_node_ids.emplace_back(transition.GetDestinationNodeId());
    }

    auto metric_info = new EntityMetricInfo{entity_id, name};
    return memory.SelectTransitionNodeId(sequences.top()->GetSequenceId(), sequences.top()->GetCurrentNodeId(),
        valid_node_ids, *metric_info);
}

// ═══════════════════════════════════════════════════════════════════════════
// PRECONDITION VALIDATION
// ═══════════════════════════════════════════════════════════════════════════

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
    ZoneScoped;

    if (!preconditions || preconditions->empty())
    {
        return true;
    }

    for (const auto& precondition : *preconditions)
    {
        StateOperationContext context(this, other);
        if (!StateEvaluator().ProcessStateOperation(precondition, context))
        {
            return false;
        }
    }
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// INTERRUPTION HANDLING
// ═══════════════════════════════════════════════════════════════════════════

void BehavioralEntity::ProcessInterruption(int32_t interruption_id)
{
    if (is_processing)
    {
        Logger().LogInfo("Entity " + std::to_string(entity_id) + " queueing interruption " +
            std::to_string(interruption_id), "ProcessInterruption");

        pending_interruptions.push(interruption_id);
        return;
    }

    ProcessInterruptionImmediate(interruption_id);
}

void BehavioralEntity::ProcessPendingInterruptions()
{
    Logger().LogInfo("Processing " + std::to_string(pending_interruptions.size()) + " queued interruptions "
        "for entity" + std::to_string(entity_id), "ProcessPendingInterruptions");

    while (!pending_interruptions.empty())
    {
        auto interruption_id = pending_interruptions.front();
        pending_interruptions.pop();

        ProcessInterruptionImmediate(interruption_id);
    }
}

void BehavioralEntity::ProcessInterruptionImmediate(int32_t interruption_id)
{
    // Check handler exists
    if (!interruption_handlers.contains(interruption_id))
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::INTERRUPTION_NOT_FOUND,
            .interruption_id = interruption_id,
            .additional_info = "ProcessInterruptionImmediate"
        });

        return;
    }

    auto sequence = interruption_handlers.at(interruption_id);

    Logger().LogInfo("Will process interruption " + std::to_string(interruption_id) + " with sequence " +
        std::to_string(sequence->GetSequenceId()) + " for entity: " + std::to_string(entity_id),
        "ProcessInterruptionImmediate");

    // Context preservation
    if (sequences.top()->GetSequenceState() == SequenceState::WAITING_FOR_ACTION)
    {
        auto action = ContentProvider().GetActionById(current_action_id);
        if (!action)
        {
            HandleRuntimeFailure({
                .reason = RuntimeFailureReason::ACTION_NOT_FOUND,
                .action_id = current_action_id,
                .additional_info = "ProcessInterruptionImmediate"
            });

            return;
        }

        FrameworkEntity* target_entity = nullptr;
        if (current_action_target_id >= 0)
        {
            target_entity = EntityQuery().GetEntityFromId(current_action_target_id);
        }

        ApplyActionEffects(action->GetInterruptionEffects(), target_entity);

        if (action->GetInterruptionBehavior() == InterruptionBehaviorType::RESUMABLE)
        {
            memory.CreateInterruptionMemory(current_action_id, sequences.top()->GetSequenceId(),
                sequences.top()->GetCurrentNodeId(), current_action_target_id, TimeManager().GetCurrentTime());

            // Invalidate action token to reject late callbacks of complete action
            current_action_token++;
        }
    }

    // Sequence State Management
    sequences.top()->SetSequenceState(SequenceState::INTERRUPTED);

    // Response sequence activation
    sequences.push(sequence->CreateInstance());
}

void BehavioralEntity::HandleInterruptionRecovery()
{
    ZoneScoped;

    auto currentSequenceId = sequences.top()->GetSequenceId();

    Logger().LogInfo("Handling interruption recovery for entity: " + std::to_string(entity_id) + " in sequence: " +
        std::to_string(currentSequenceId), "HandleInterruptionRecovery");

    // Check if current node exists
    auto current_node = sequences.top()->FindCurrentNode();
    if (!current_node)
    {
        HandleRuntimeFailure({
            .reason = RuntimeFailureReason::NODE_NOT_FOUND,
            .node_id = sequences.top()->GetCurrentNodeId(),
            .additional_info = "HandleInterruptionRecovery - current node not found"
        });

        return;
    }

    auto action_node = dynamic_cast<const ActionSequenceNode*>(current_node);
    if (!action_node)
    {
        Logger().LogInfo("Interrupted sequence" + std::to_string(currentSequenceId) + " for entity " +
            std::to_string(entity_id) + " was not at an action node. Continuing processing",
            "HandleInterruptionRecovery");

        sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
        return;
    }


    // Check if sequence was executing action
    current_action_id = action_node->GetTargetActionId();
    auto action = ContentProvider().GetActionById(current_action_id);
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
        AttemptActionResumption(action);
        return;
    }

    Logger().LogInfo("Action " + std::to_string(current_action_id) + " is non-resumable for entity" +
        std::to_string(entity_id), "HandleInterruptionRecovery");

    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
}

void BehavioralEntity::AttemptActionResumption(const std::shared_ptr<Action>& action)
{

    Logger().LogInfo("Attempting to resume action " + std::to_string(current_action_id) + " from interruption "
        "for entity" + std::to_string(entity_id), "AttemptActionResumption");

    auto interruption_memory = memory.FindInterruptionMemory(current_action_id, sequences.top()->GetSequenceId(),
                sequences.top()->GetCurrentNodeId());

    if (!interruption_memory)
    {
        Logger().LogInfo("No interruption memory exists for action " + std::to_string(current_action_id) +
            " for entity " + std::to_string(entity_id), "AttemptActionResumption");

        sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
        return;
    }

    if (ValidateResumptionContext(action, interruption_memory->GetInterruptedTargetEntityId()))
    {
        Logger().LogInfo("Resuming action " + std::to_string(action->GetActionId()) + " with saved target "
            "entity " + std::to_string(interruption_memory->GetInterruptedTargetEntityId()),
            "AttemptActionResumption");

        ResumeActionWithSavedContext(action, interruption_memory);
        memory.RemoveInterruptionMemory(interruption_memory);
        return;
    }

    Logger().LogInfo("Resumption context invalid for action " + std::to_string(action->GetActionId()) +
        ", attempting fresh execution", "AttemptActionResumption");

    memory.RemoveInterruptionMemory(interruption_memory);
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
}

bool BehavioralEntity::ValidateResumptionContext(const std::shared_ptr<Action>& action, int32_t target_entity_id)
{
    if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::SELF), nullptr))
    {
        return false;
    }

    if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::ENVIRONMENT), nullptr))
    {
        return false;
    }

    if (action->GetRequiresTargetEntity())
    {
        auto target_entity = EntityQuery().GetEntityFromId(target_entity_id);
        if (!target_entity)
        {
            return false;
        }

        if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::ENTITY), target_entity))
        {
            return false;
        }

        if (!EvaluatePreconditions(action->GetPreconditionsForTarget(StateOperationTarget::DISTANCE_TO_ENTITY), target_entity))
        {
            return false;
        }
    }

    return true;
}

void BehavioralEntity::ResumeActionWithSavedContext(const std::shared_ptr<Action>& action,
    const InterruptionMemory* interruption_memory)
{
    FrameworkEntity* target_entity = nullptr;
    if (action->GetRequiresTargetEntity())
    {
        target_entity = EntityQuery().GetEntityFromId(interruption_memory->GetInterruptedTargetEntityId());
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

    InitiateActionExecution(action, target_entity);
}

// ═══════════════════════════════════════════════════════════════════════════
// FAILURE RECOVERY & TRANSITION COMPLETION
// ═══════════════════════════════════════════════════════════════════════════

void BehavioralEntity::HandleNodeExecutionCompletion()
{
    ZoneScoped;

    if (!sequences.empty() && sequences.top()->GetSequenceState() == SequenceState::INTERRUPTED)
    {
        Logger().LogInfo(
            "Node execution completion aborted - entity " + std::to_string(entity_id) +
            " was interrupted",
            "HandleNodeExecutionCompletion"
        );
        return;
    }

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

    if (!sequences.empty() && sequences.top()->GetSequenceState() == SequenceState::INTERRUPTED)
    {
        Logger().LogWarning(
            "Entity " + std::to_string(entity_id) +
            " interrupted during transition selection. Aborting node completion.",
            "HandleNodeExecutionCompletion"
        );
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
    memory.CreateTransitionMemory(sequences.top()->GetSequenceId(), selected_node_id.value(),
        TimeManager().GetCurrentTime());

    sequences.top()->FindCurrentNode()->ResetCompletion();
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
    fallback_attempt_count = 0;
}

void BehavioralEntity::HandleSubsequenceCompletion()
{
    ZoneScoped;

    Logger().LogInfo("Finished running subsequence for entity" + std::to_string(entity_id),
        "HandleSubsequenceCompletion");

    sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
    HandleNodeExecutionCompletion();
}


void BehavioralEntity::HandleSequenceFailure()
{
    ZoneScoped;

    fallback_attempt_count++;
    if (fallback_attempt_count >= MAX_FALLBACK_ATTEMPTS)
    {
        Logger().LogError("Entity " + std::to_string(entity_id) + " exceeded max fallback attempts, halting",
            "HandleSequenceFailure");

        // is halted is turned to true to avoid updating this character
        is_halted = true;
        return;
    }

    auto currentSequenceId = sequences.top()->GetSequenceId();

    Logger().LogInfo("Handling sequence (" + std::to_string(currentSequenceId) + ") failure for entity: " +
        std::to_string(entity_id), "HandleSequenceFailure");

    memory.ClearSequenceInterruptionMemories(currentSequenceId);
    sequences.top()->ResetCurrentNodeToEntry();
    sequences.pop();

    if (fallback_sequences.empty())
    {
        Logger().LogError("No fallback sequences available for entity " +
                       std::to_string(entity_id), "HandleSequenceFailure");

        return;
    }

    auto fallback_sequence_template = fallback_sequences[rand() % fallback_sequences.size()];
    auto fallback_instance = fallback_sequence_template->CreateInstance();

    Logger().LogInfo("Entity with id: " + std::to_string(entity_id) + " will now follow fallback sequence with "
        "id: " + std::to_string(fallback_instance->GetSequenceId()),
        "HandleSequenceFailure");

    sequences.push(fallback_instance);
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
            reason_str = "Action not found " + (context.action_id > -1 ? " (ID: " + std::to_string(context.action_id) +
                ")" : "");
            break;
        case RuntimeFailureReason::ENTITY_NOT_FOUND:
            reason_str = "Entity not found " + (context.entity_id > -1 ? " (ID: " + std::to_string(context.entity_id) +
                ")" : "");
            break;
        case RuntimeFailureReason::SEQUENCE_NOT_FOUND:
            reason_str = "Sequence not found " + (context.sequence_id > -1 ? " (ID: " +
                std::to_string(context.sequence_id) + ")" : "");
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
            reason_str = "Interruption not found " + (context.interruption_id > -1 ? " (ID: " +
                std::to_string(context.interruption_id) + ")" : "");
            break;
    }

    Logger().LogError("Action execution failed for entity " + std::to_string(entity_id) + ": " + reason_str +
        (context.additional_info.empty() ? "" : " - " + context.additional_info),
        "HandleRuntimeFailure");

    sequences.top()->SetSequenceState(SequenceState::FAILED);

    // Stop processing if context indicates we should
    if (context.should_stop_processing) {
        is_processing = false;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// STATE ACCESSORS (for inspection/debugging)
// ═══════════════════════════════════════════════════════════════════════════

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
        Logger().LogWarning("Sequence with id: " + std::to_string(sequence_id) + " is not in the fallback sequences",
         "GetFallbackSequenceById");
        return nullptr;
    }

    return *iterator;
}

std::shared_ptr<Sequence> BehavioralEntity::FindInterruptionHandler(int32_t interruption_id) const
{
    auto iterator = interruption_handlers.find(interruption_id);
    if (iterator == interruption_handlers.end())
    {
        Logger().LogWarning("FrameworkEntity[" + std::to_string(entity_id) + "]: Interruption id: " +
            std::to_string(interruption_id) + " not found", "FindInterruptionHandler");

        return nullptr;
    }

    return iterator->second;
}