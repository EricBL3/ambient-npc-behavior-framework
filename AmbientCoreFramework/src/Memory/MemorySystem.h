/*
 * MemorySystem.h
 *
 * Core memory management for ambient character behavioral variety
 *
 * Author: Eric Buitrón López
 * Created: 8/13/2025
*/

#pragma once
#include <vector>
#include <string>

#include "ActionMemory.h"
#include "InterruptionMemory.h"
#include "TransitionMemory.h"

class MemorySystem {
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    int max_transition_memories;
    int max_action_memories;
    int max_interruption_memories;

    std::vector<TransitionMemory> transition_memories;
    std::vector<ActionMemory> action_memories;
    std::vector<InterruptionMemory> interruption_memories;

    // =============================================================================
    // UTILITY HELPERS
    // =============================================================================

    void LogError(const std::string& message) const;
    int SelectRandomFromVector(const std::vector<int>& options) const;

    // =============================================================================
    // CAPACITY MANAGEMENT
    // =============================================================================

    void EnforceMaxTransitionMemories();
    void EnforceMaxActionMemories();
    void EnforceMaxInterruptionMemories();

    // =============================================================================
    // MEMORY REMOVAL HELPERS
    // =============================================================================

    void RemoveExistingTransitionMemory(int target_node_id);
    void RemoveExistingActionMemory(int action_id, int target_entity_id);

    // =============================================================================
    // RECENCY SELECTION HELPERS
    // =============================================================================

    int FindOldestTransitionNode(const std::vector<int>& node_ids) const;
    int FindOldestActionEntity(int action_id, const std::vector<int>& entity_ids) const;
    std::vector<int> FindUnusedTransitionNodes(const std::vector<int>& node_ids) const;
    std::vector<int> FindUnusedActionEntities(int action_id, const std::vector<int>& entity_ids) const;

public:
    // =============================================================================
    // CONSTRUCTION & CONFIGURATION
    // =============================================================================

    MemorySystem(int max_transitions, int max_actions, int max_interruptions);

    int GetMaxTransitionMemories() const;
    int GetMaxActionMemories() const;
    int GetMaxInterruptionMemories() const;

    void SetMaxTransitionMemories(int max_transitions);
    void SetMaxActionMemories(int max_actions);
    void SetMaxInterruptionMemories(int max_interruptions);

    // =============================================================================
    // MEMORY UPDATE OPERATIONS
    // =============================================================================

    bool UpdateTransitionMemory(int target_node_id, int current_time);
    bool UpdateActionMemory(int action_id, int target_entity_id, int current_time);
    bool UpdateInterruptionMemory(int action_id, int sequence_id, int node_id, int entity_id, int current_time);

    // =============================================================================
    // MEMORY SEARCH OPERATIONS
    // =============================================================================

    const TransitionMemory* FindTransitionMemory(int target_node_id) const;
    const ActionMemory* FindActionMemory(int action_id, int target_entity_id) const;
    const InterruptionMemory* FindInterruptionMemory(int action_id, int sequence_id, int node_id) const;

    // =============================================================================
    // BEHAVIORAL SELECTION (Framework's Core Functionality)
    // =============================================================================

    // Returns the node_id that has been used less recently.
    int GetLeastRecentlyVisitedNode(const std::vector<int>& node_ids) const;

    // Returns the entity_id that has been used less recently with the action_id.
    int GetLeastRecentlyUsedEntityForAction(int action_id, const std::vector<int>& entity_ids) const;

    // =============================================================================
    // MEMORY CLEANUP OPERATIONS
    // =============================================================================

    void ClearSequenceInterruptionMemories(int sequence_id);
    bool RemoveInterruptionMemory(int action_id, int sequence_id, int node_id);
    void ClearAllMemories();


    // =============================================================================
    // DIAGNOSTIC & MONITORING
    // =============================================================================
    size_t GetTransitionMemoryCount() const;
    size_t GetActionMemoryCount() const;
    size_t GetInterruptionMemoryCount() const;
};

