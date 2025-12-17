
#pragma once
#include <queue>
#include <stack>
#include <unordered_map>

#include "RuntimeFailureReason.h"
#include "FrameworkEntity.h"
#include "behavior/Sequence.h"
#include "services/interfaces/IContentProvider.h"
#include "services/interfaces/IEntityQuery.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStartCharacterActionProvider.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"
#include "memory/MemorySystem.h"


namespace AmbientCharacterBehavior {
/**
 * @brief Entities that have decision-making capabilities and behavioral systems (Characters)
 */
class BehavioralEntity : public FrameworkEntity {
private:
    ContentRegistryServices& services;

    MemorySystem memory;
    std::shared_ptr<Sequence> main_sequence;
    std::stack<std::shared_ptr<Sequence>> sequences;
    std::vector<std::shared_ptr<Sequence>> fallback_sequences;
    std::unordered_map<int32_t, std::shared_ptr<Sequence>> interruption_handlers;

    int32_t current_action_target_id;
    bool is_processing;
    int32_t current_action_id;
    int64_t current_action_token;
    int32_t fallback_attempt_count;
    const int32_t MAX_FALLBACK_ATTEMPTS = 3;
    bool is_halted;
    std::queue<int32_t> pending_interruptions;

    FoundationServices& FoundationServices() const
    {
        return services.behavioral_evaluation.simulation_state.data_access.foundation;
    }

    ILogger& Logger() const
    {
        return FoundationServices().logger;
    }

    ITimeManager& TimeManager() const
    {
        return FoundationServices().time_manager;
    }

    IStartCharacterActionProvider& ActionProvider() const
    {
        return FoundationServices().start_character_action_provider;
    }

    IStateOperationEvaluator& StateEvaluator() const
    {
        return services.behavioral_evaluation.state_operation_evaluator;
    }

    IContentProvider& ContentProvider() const
    {
        return services.content_provider;
    }

    IEntityQuery& EntityQuery() const
    {
        return services.entity_query;
    }

public:
    /**
     * @throw std::invalid_argument if entity_id or current_location_id < 0.
     */
    explicit BehavioralEntity(
        ContentRegistryServices& services,
        void* entity_handle,
        int32_t entity_id,
        int32_t max_transition_memories,
        int32_t max_action_memories,
        int32_t max_interruption_memories,
        std::string name = ""
    );


    MemorySystem& GetMemorySystem() { return memory; }

    int64_t GetCurrentActionToken() const { return current_action_token; }

    int32_t GetCurrentActionTargetId() const { return current_action_target_id; }

    int32_t GetCurrentActionId() const { return current_action_id; }

    bool IsProcessing() const { return is_processing; }

    bool CanUpdate() const;

    void SetMainSequence(const std::shared_ptr<Sequence> &new_sequence);
    const std::shared_ptr<Sequence>& GetMainSequence() const { return main_sequence; }

    void AddFallbackSequence(const std::shared_ptr<Sequence> &new_sequence);
    bool HasFallbackSequence(int32_t sequence_id) const;
    std::shared_ptr<Sequence> GetFallbackSequenceById(int32_t sequence_id) const;

    void AddInterruptionHandler(int32_t interruption_id, const std::shared_ptr<Sequence> &handler);
    std::shared_ptr<Sequence> FindInterruptionHandler(int32_t interruption_id) const;

    void ExecuteCurrentSequence();

    void ProcessInterruption(int32_t interruption_id);

    void CompleteAction(int32_t action_id, int64_t action_token);

private:
    void HandleEmptySequences();
    void HandleSequenceStartup();

    void ExecuteSequenceStep(SequenceState sequence_state);
    void ProcessCurrentNode();

    void ExecuteCurrentNode(const SequenceNode* current_node);

    void ExecuteActionNode(const SequenceNode* current_node);
    std::shared_ptr<Action> LookupActionFromCurrentNode(const SequenceNode* current_node) const;
    void InitiateActionExecution(const std::shared_ptr<Action>& action, FrameworkEntity* target_entity);

    void ExecuteNestedSequenceNode(const SequenceNode* current_node);
    void ExecuteEndSequenceNode(const SequenceNode* current_node);

    FrameworkEntity* GetActionTargetEntity(const std::shared_ptr<Action>& action);
    void ApplyActionEffects(const std::vector<StateOperation> & effects, FrameworkEntity* target_entity);

    void HandleSubsequenceCompletion();
    void HandleNodeExecutionCompletion();
    void HandleSequenceFailure();

    void HandleInterruptionRecovery();
    void AttemptActionResumption(const std::shared_ptr<Action>& action);
    bool ValidateResumptionContext(const std::shared_ptr<Action>& action, int32_t target_entity_id);
    void ResumeActionWithSavedContext(const std::shared_ptr<Action>& action, const InterruptionMemory* interruption_memory);

    bool CompletedCurrentAction(int32_t action_id, int64_t action_token) const;
    void ApplyCompletionEffects(int32_t action_id);

    struct PreconditionValidation {
        bool precondition_passed;
        std::optional<StateOperationTarget> failed_target;

        bool Passed() const { return precondition_passed; }
    };

    PreconditionValidation ValidateActionPreconditions(const std::shared_ptr<Action>& action,
        FrameworkEntity* target_entity = nullptr);

    bool EvaluatePreconditions(const std::vector<StateOperation>* preconditions, FrameworkEntity* other);
    std::optional<int32_t> GetNodeIdForNextTransition();

    void HandleRuntimeFailure(const RuntimeFailureContext& context);

    void ProcessPendingInterruptions();
    void ProcessInterruptionImmediate(int32_t interruption_id);
};
}
