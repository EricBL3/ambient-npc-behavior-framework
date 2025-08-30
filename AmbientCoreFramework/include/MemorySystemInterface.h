/**
 * @file MemorySystemInterface.h
 * @brief Public C interface for MemorySystem operations
 * @author Eric Buitrón López
 * @date 8/15/2025
 */

#pragma once
#include <cstdint>

#ifdef _WIN32
    #ifdef AmbientCoreFramework_EXPORTS
        #define AmbientCoreFramework_API __declspec(dllexport)
    #else
        #define AmbientCoreFramework_API __declspec(import)
    #endif

#else
    #define AmbientCoreFramework_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /// Handle type for the memory system instances
    typedef void* MemorySystemHandle;

    // =============================================================================
    // MEMORY SYSTEM LIFECYCLE
    // =============================================================================

    /**
     * @brief Creates a MemorySystem instance with specified capacities
     * @param max_transitions Maximum transition memories (recommended: 5-10)
     * @param max_actions Maximum action memories (recommended: 10-20)
     * @param max_interruptions Maximum interruption memories (recommended: 3-5)
     * @return Handle to MemorySystem instance, or NULL on failure
     */
    AmbientCoreFramework_API MemorySystemHandle CreateMemorySystem(
        int32_t max_transitions,
        int32_t max_actions,
        int32_t max_interruptions
    );

    /**
     * @brief Destroys a specific MemorySystem instance
     * @param handle Handle to the MemorySystem instance
     */
    AmbientCoreFramework_API void DestroyMemorySystem(MemorySystemHandle handle);

    // =============================================================================
    // BEHAVIORAL SELECTION
    // =============================================================================

    /**
     * @brief Selects least recently visited node for behavioral variety
     * @param handle Handle to the MemorySystem instance
     * @param node_ids Array of available node options
     * @param count Number of nodes in the array
     * @return Selected node ID, or -1 on error
     */
    AmbientCoreFramework_API int32_t GetLeastRecentlyVisitedNode(
        MemorySystemHandle handle,
        int32_t* node_ids,
        int32_t count
    );

    /**
     * @brief Selects least recently used entity for specific action
     * @param handle Handle to the MemorySystem instance
     * @param action_id Action being performed
     * @param entity_ids Array of available entity options
     * @param count Number of entities in the array
     * @return Selected entity ID, or -1 on error
     */
    AmbientCoreFramework_API int32_t GetLeastRecentlyUsedEntityForAction(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t* entity_ids,
        int32_t count
    );

    // =============================================================================
    // MEMORY UPDATES
    // =============================================================================

    /**
     * @brief Records a transition decision to prevent future repetition
     * @param handle Handle to the MemorySystem instance
     * @param target_node_id Node that was selected
     * @param current_time Simulation timestamp
     * @return true on success, false on failure
     */
    AmbientCoreFramework_API bool UpdateTransitionMemory(
        MemorySystemHandle handle,
        int32_t target_node_id,
        int64_t current_time
    );

    /**
     * @brief Records an action execution to prevent future repetition
     * @param handle Handle to the MemorySystem instance
     * @param action_id Action that was performed
     * @param target_entity_id Entity that was used
     * @param current_time Simulation timestamp
     * @return true on success, false on failure
     */
    AmbientCoreFramework_API bool UpdateActionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t target_entity_id,
        int64_t current_time
    );

    /**
     * @brief Records interruption context for potential resumption
     * @param handle Handle to the MemorySystem instance
     * @param action_id Action that was interrupted
     * @param sequence_id Sequence that was executing
     * @param node_id Sequence node that was interrupted
     * @param entity_id Entity involved (-1 if none)
     * @param current_time Simulation timestamp
     * @return true on success, false on failure
     */
    AmbientCoreFramework_API bool UpdateInterruptionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t sequence_id,
        int32_t node_id,
        int32_t entity_id,
        int64_t current_time
    );

    // =============================================================================
    // MEMORY SEARCH (For marshalling complexity tests)
    // =============================================================================

    /**
     * @brief Finds transition memory and returns its timestamp
     * @param handle Handle to the MemorySystem instance
     * @param target_node_id Node to search for
     * @param[out] out_timestamp Pointer to receive the timestamp (if found)
     * @return true on success, false on failure
     *
     * Used for testing complex marshalling scenarios with output parameters.
     */
    AmbientCoreFramework_API bool FindTransitionMemory(
        MemorySystemHandle handle,
        int32_t target_node_id,
        int64_t* out_timestamp
    );

    /**
     * @brief Finds action memory and returns its timestamp
     * @param handle Handle to the MemorySystem instance
     * @param action_id Action to search for
     * @param target_entity_id Entity to search for
     * @param[out] out_timestamp Pointer to receive the timestamp (if found)
     * @return true on success, false on failure
     */
    AmbientCoreFramework_API bool FindActionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t target_entity_id,
        int64_t* out_timestamp
    );

    /**
     * @brief Finds interruption memory and returns its data
     * @param handle Handle to the MemorySystem instance
     * @param action_id Action to search for
     * @param sequence_id Sequence to search for
     * @param node_id Node to search for
     * @param[out] out_entity_id Pointer to receive entity ID (if found)
     * @param[out] out_timestamp Pointer to receive timestamp (if found)
     * @return true on success, false on failure
     */
    AmbientCoreFramework_API bool FindInterruptionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t sequence_id,
        int32_t node_id,
        int32_t* out_entity_id,
        int64_t* out_timestamp
    );

    // =============================================================================
    // MEMORY CLEANUP
    // =============================================================================

    /**
     * @brief Clears all memories from the current instance
     * @param handle Handle to the MemorySystem instance
     */
    AmbientCoreFramework_API void ClearAllMemories(MemorySystemHandle handle);

    /**
     * @brief Removes all interruption memories for a specific sequence
     * @param handle Handle to the MemorySystem instance
     * @param sequence_id Sequence whose memories should be cleared
     */
    AmbientCoreFramework_API void ClearSequenceInterruptionMemories(
        MemorySystemHandle handle,
        int32_t sequence_id
    );

    // =============================================================================
    // DIAGNOSTIC
    // =============================================================================

    /**
     * @brief Gets current number of stored transition memories
     * @param handle Handle to the MemorySystem instance
     * @return Count of transition memories
     */
    AmbientCoreFramework_API int32_t GetTransitionMemoryCount(MemorySystemHandle handle);

    /**
     * @brief Gets current number of stored action memories
     * @param handle Handle to the MemorySystem instance
     * @return Count of action memories
     */
    AmbientCoreFramework_API int32_t GetActionMemoryCount(MemorySystemHandle handle);

    /**
     * @brief Gets current number of stored interruption memories
     * @param handle Handle to the MemorySystem instance
     * @return Count of interruption memories
     */
    AmbientCoreFramework_API int32_t GetInterruptionMemoryCount(MemorySystemHandle handle);

#ifdef __cplusplus
}
#endif