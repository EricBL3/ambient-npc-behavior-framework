#pragma once
#include <deque>
#include <optional>
#include <random>
#include <vector>

#include "ActionMemory.h"
#include "InterruptionMemory.h"
#include "TransitionMemory.h"
#include "services/interfaces/ILogger.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Central coordinator for all character memory types and behavioral variety generation
 *
 * The MemorySystem serves three primary functions:
 * 1. Storage: Maintains bounded collections of transition, action, and interruption memories
 * 2. Selection: Implements exploration-exploitation algorithms for decision-making
 * 3. Management: Enforces capacity limits and handles memory lifecycle
 *
 * Design Philosophy:
 * - Uses std::deque for O(1) front-removal in FIFO pattern while still allowing type specific matching criteria.
 * - Provides separate methods for each memory type to maintain type safety
 *
 * Memory Capacity:
 * Each memory has independent configurable capacity limits. When limits are exceeded, oldest memories are removed (FIFO).
 * This creates a "forgetting" effect where old decisions eventually become "unused" again, preventing indefinite avoidance
 * patterns.
 */
class MemorySystem {
private:

    // =============================================================================
    // Capacity Limits
    // =============================================================================

    /**
     * @invariant max_transition_memories > 0
     */
    int32_t max_transition_memories;

    /**
     * @invariant max_action_memories > 0
     */
    int32_t max_action_memories;

    /**
     * @invariant max_interruption_memories > 0
     */
    int32_t max_interruption_memories;

    // =============================================================================
    // Memory Storage
    // =============================================================================

    std::deque<TransitionMemory> transition_memories;
    std::deque<ActionMemory> action_memories;
    std::deque<InterruptionMemory> interruption_memories;

    // =============================================================================
    // Dependencies
    // =============================================================================

    ILogger& logger;

    // Random number generator for tie-breaking
    std::mt19937 rng;

public:
    // =============================================================================
    // Construction
    // =============================================================================

    /**
     * @brief Construct a memory system with specified capacity limits
     * @param max_transitions Maximum number of transition memories to store (must be > 0)
     * @param max_actions Maximum number of action memories to store (must be > 0)
     * @param max_interruptions Maximum number of interruption memories to store (must be > 0)
     * @param logger Logger for memory operations
     */
    explicit MemorySystem(int32_t max_transitions, int32_t max_actions, int32_t max_interruptions, ILogger& logger);

    // =============================================================================
    // Capacity Configuration
    // =============================================================================

    /**
     * @brief Updates the maximum transition memory capacity
     *
     * If the new limit is lower than the current memory count, excess memories are removed from the front (oldest).
     * @param max_transitions New capacity (must be > 0 and different from current capacity)
     */
    void ConfigureMaxTransitionMemories(int32_t max_transitions);

    /**
     * @brief Updates the maximum transition memory capacity
     *
     * If the new limit is lower than the current memory count, excess memories are removed from the front (oldest).
     * @param max_actions New capacity (must be > 0 and different from current capacity)
     */
    void ConfigureMaxActionMemories(int32_t max_actions);

    /**
     * @brief Updates the maximum transition memory capacity
     *
     * If the new limit is lower than the current memory count, excess memories are removed from the front (oldest).
     * @param max_interruptions New capacity (must be > 0 and different from current capacity)
     */
    void ConfigureMaxInterruptionMemories(int32_t max_interruptions);

    // =============================================================================
    // Memory Creation
    // =============================================================================

    /**
     * @brief Create a new transition memory
     *
     * If a memory already exists for this (sequence_id, node_id) pair, it is removed before adding the new one. This ensures
     * each memory is recorded only once with its most recent timestamp.
     *
     * @param sequence_id Sequence containing the node
     * @param target_node_id Node that was visited
     * @param current_time Timestamp of the memory
     * @return true if creation succeeded, false if validation failed
     */
    bool CreateTransitionMemory(int32_t sequence_id, int32_t target_node_id, int64_t current_time);

    /**
     * @brief Create a new action memory
     *
     * If a memory already exists for this (action_id, target_entity_id) pair, it is removed before adding the new one.
     * This ensures each memory is recorded only once with its most recent timestamp.
     *
     * @param action_id Action being performed
     * @param target_entity_id Target entity that will be used for the action.
     * @param current_time Timestamp of the memory
     * @return true if creation succeeded, false if validation failed
     */
    bool CreateActionMemory(int32_t action_id, int32_t target_entity_id, int64_t current_time);

    /**
     * @brief Create a new interruption memory
     *
     * If a memory already exists for this (action_id, sequence_id, node_id) triple, it is removed before adding the new one.
     * This ensures each memory is recorded only once with its most recent timestamp.
     *
     * @param action_id Action being performed
     * @param sequence_id Sequence being executed
     * @param node_id Node that was being processed
     * @param entity_id Target entity that was being used for the action.
     * @param current_time Timestamp of the memory
     * @return true if creation succeeded, false if validation failed
     */
    bool CreateInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id, int32_t entity_id, int64_t current_time);

    // =============================================================================
    // Exploration-Exploitation Selection
    // =============================================================================

    /**
     * @brief Select a transition node using exploration-exploitation strategy
     *
     * Implements the memory-driven exploration-exploitation algorithm:
     * 1. Exploration: If any nodes are unused (not in memory), randomly select from them
     * 2. Exploitation: Otherwise, select from least recently used node(s)
     * 3. Tie-breaking: Randomly select among equal-priority options
     *
     * This creates behavioral variety by:
     * - Encouraging exploration of new paths (unused nodes)
     * - Distributing usage over time (least recently used selection)
     * - Preventing systematic bias (random tie-breaking)
     *
     * @param sequence_id Current sequence (for memory lookup)
     * @param valid_node_ids Nodes that satisfy the transition preconditions
     * @return The selected node id, or std::nullopt if valid_node_ids is empty
     */
    [[nodiscard]]
    std::optional<int32_t> SelectTransitionNodeId(int32_t sequence_id, const std::vector<int32_t>& valid_node_ids);

    /**
     * @brief Select an action target entity using exploration-exploitation strategy
     *
     * Same algorithm SelectTransitionNodeId but for entity selection.
     * @param action_id Current action (for memory lookup)
     * @param valid_entity_ids Entities that satisfy preconditions of the action
     * @return Selected entity id, or std::nullopt if valid_entity_ids is empty
     */
    [[nodiscard]]
    std::optional<int32_t> SelectActionEntityId(int32_t action_id, const std::vector<int32_t>& valid_entity_ids);

    // =============================================================================
    // Memory Query
    // =============================================================================

    [[nodiscard]]
    const TransitionMemory* FindTransitionMemory(int32_t sequence_id, int32_t target_node_id) const;

    [[nodiscard]]
    const ActionMemory* FindActionMemory(int32_t action_id, int32_t target_entity_id) const;

    [[nodiscard]]
    const InterruptionMemory* FindInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id) const;

    // =============================================================================
    // Interruption Memory Management
    // =============================================================================

    /**
     * @brief Remove a specific interruption memory
     * @return true if memory was found and removed
     */
    bool RemoveInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id);
    bool RemoveInterruptionMemory(const InterruptionMemory* memory);

    /**
     * @brief Clear all interruption memories for a sequence
     *
     * Called when a sequence fails to ensure clean restart without attempting to resume actions from the failed sequence.
     */
    void ClearSequenceInterruptionMemories(int32_t sequence_id);

    /**
     * @brief Clear all memories (used during initialization)
     */
    void ClearAllMemories();

    // =============================================================================
    // Memory Metrics
    // =============================================================================

    [[nodiscard]]
    int32_t GetMaxTransitionMemories() const { return max_transition_memories; }

    [[nodiscard]]
    int32_t GetMaxActionMemories() const { return max_action_memories; }

    [[nodiscard]]
    int32_t GetMaxInterruptionMemories() const { return max_interruption_memories; }

    [[nodiscard]]
    size_t GetTransitionMemoryCount() const { return transition_memories.size(); }

    [[nodiscard]]
    size_t GetActionMemoryCount() const { return action_memories.size(); }

    [[nodiscard]]
    size_t GetInterruptionMemoryCount() const { return interruption_memories.size(); }

private:
    // =============================================================================
    // Capacity Enforcement
    // =============================================================================

    /**
     * @brief Remove the oldest transition memories if over capacity
     *
     * Uses pop_front() for O(1) removal. Called after each memory creation.
     */
    void EnforceMaxTransitionMemories();

    /**
     * @brief Remove the oldest action memories if over capacity
     *
     * Uses pop_front() for O(1) removal. Called after each memory creation.
     */
    void EnforceMaxActionMemories();

    /**
     * @brief Remove the oldest interruption memories if over capacity
     *
     * Uses pop_front() for O(1) removal. Called after each memory creation.
     */
    void EnforceMaxInterruptionMemories();

    // =============================================================================
    // Memory Deduplication
    // =============================================================================

    /**
     * @brief Remove existing transition memory before adding new one
     *
     * Ensures each (sequence_id, node_id) pair appears at most once, with the most recent timestamp.
     * Called before adding new memories.
     */
    void RemoveExistingTransitionMemory(int32_t sequence_id, int32_t target_node_id);

    /**
     * @brief Remove existing action memory before adding new one
     *
     * Ensures each (action_id, target_entity_id) pair appears at most once, with the most recent timestamp.
     * Called before adding new memories.
     */
    void RemoveExistingActionMemory(int32_t action_id, int32_t target_entity_id);

    // =============================================================================
    // Random Selection Helper
    // =============================================================================

    /**
     * @brief Generate random index for tie-breaking
     * @param max_exclusive Upper bound (exclusive)
     * @return Random index in [0, max_exclusive)
     */
    int32_t GetRandomIndex(int32_t max_exclusive);
};

}
