
#pragma once
#include <stack>
#include <unordered_map>

#include "FrameworkEntity.h"
#include "behavior/Sequence.h"
#include "interfaces/IContentProvider.h"
#include "interfaces/IEntityQuery.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStartCharacterActionProvider.h"
#include "interfaces/IStateOperationEvaluator.h"
#include "interfaces/ITimeManager.h"
#include "memory/MemorySystem.h"


namespace AmbientCharacterBehavior {
/**
 * @brief Entities that have decision-making capabilities and behavioral systems (Characters)
 */
class BehavioralEntity : public FrameworkEntity {
private:
    ILogger& logger;

    ITimeManager& time_manager;

    IContentProvider& content_provider;

    IEntityQuery& entity_query;

    IStartCharacterActionProvider& start_character_action_provider;

    IStateOperationEvaluator& state_operation_evaluator;

    MemorySystem memory;

    std::shared_ptr<Sequence> main_sequence;

    std::stack<std::shared_ptr<Sequence>> sequences;

    std::vector<std::shared_ptr<Sequence>> fallback_sequences;

    std::unordered_map<int32_t, std::shared_ptr<Sequence>> interruption_handlers;

    int32_t current_action_target_id;

    bool is_processing;

    int32_t current_action_id;
    int64_t current_action_token;

public:
    /**
     * @throw std::invalid_argument if entity_id or current_location_id < 0.
     */
    explicit BehavioralEntity(ILogger& logger, ITimeManager& time_manager, IStartCharacterActionProvider& action_provider,
        IContentProvider& content_provider, IEntityQuery& entity_query,
        IStateOperationEvaluator& state_operation_evaluator, void* entity_handle, int32_t entity_id, int32_t max_transition_memories,
        int32_t max_action_memories, int32_t max_interruption_memories, std::string name = "") :
        FrameworkEntity(entity_handle, entity_id, std::move(name)), logger(logger), time_manager(time_manager),
        start_character_action_provider(action_provider), content_provider(content_provider), entity_query(entity_query),
        state_operation_evaluator(state_operation_evaluator),
        memory(max_transition_memories, max_action_memories, max_interruption_memories, logger),
        main_sequence(nullptr), current_action_target_id(-1), is_processing(false), current_action_token(0), current_action_id(-1) {}

    const MemorySystem& GetMemorySystem() const { return memory; }

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

    void ExecuteCurrentNode();
    void ExecuteActionNode(const SequenceNode* current_node);
    void ExecuteNestedSequenceNode(const SequenceNode* current_node);
    void ExecuteEndSequenceNode(const SequenceNode* current_node);

    FrameworkEntity* GetActionTargetEntity(const std::shared_ptr<Action>& action);
    void ApplyActionEffects(const std::vector<StateOperation> & effects, FrameworkEntity* target_entity);

    void HandleSubsequenceCompletion();
    void HandleNodeExecutionCompletion();
    void HandleSequenceFailure();
    void HandleInterruptionRecovery();

    bool CompletedCurrentAction(int32_t action_id, int64_t action_token) const;
};
}
