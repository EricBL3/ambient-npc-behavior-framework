/**
* @file MemorySystem.cpp
 * @brief Implementation of the core memory management for ambient character behavioral variety
 * @author Eric Buitrón López
 * @date 8/13/2025
 */

#include "MemorySystem.h"
#include <algorithm>
#include <iostream>

#include "utils/PerformanceTracker.h"

// =============================================================================
// CONSTRUCTION & CONFIGURATION
// =============================================================================

/**
 * @brief Constructs MemorySystem with specified capacity limits for each memory type
 * @param max_transitions Maximum transition memories (recommended: 5-10)
 * @param max_actions Maximum action memories (recommended: 10-20)
 * @param max_interruptions Maximum interruption memories (recommended: 3-5)
 *
 * @algorithm Validates and stores capacity limits, initializes empty memory collections
 * @rationale
 * - Delegates individual validation to setter methods for consistency
 * - ClearAllMemories() ensures clean initial state
 * @complexity O(1) - simple initialization
 * @datastructures Uses std::vector for each memory type (allows efficient iteration and removal)
 */
MemorySystem::MemorySystem(int max_transitions, int max_actions, int max_interruptions)
{
    SetMaxTransitionMemories(max_transitions);
    SetMaxActionMemories(max_actions);
    SetMaxInterruptionMemories(max_interruptions);

    ClearAllMemories();
}

int MemorySystem::GetMaxTransitionMemories() const
{
    return max_transition_memories;
}

int MemorySystem::GetMaxActionMemories() const
{
    return max_action_memories;
}

int MemorySystem::GetMaxInterruptionMemories() const
{
    return max_interruption_memories;
}

/**
 * @brief Sets the maximum number of transition memories with validation and enforcement
 * @param max_transitions New maximum capacity (must be > 0)
 *
 * @algorithm Validation + assignment + immediate capacity enforcement
 * @rationale
 * - Validation prevents invalid configurations that could break memory management
 * - Immediate enforcement ensures current memories comply with new limit
 * - Error logging maintains system stability while alerting to configuration issues
 * @complexity O(1) if no enforcement needed, O(k) where k = memories to remove
 * @datastructures Triggers vector manipulation if current size exceeds new limit
 */
void MemorySystem::SetMaxTransitionMemories(int max_transitions)
{
    if (max_transitions <= 0)
    {
        LogError("MemorySystem: max_transitions must be greater than 0, keeping current value");
        return;
    }

    this->max_transition_memories = max_transitions;
    transition_memories.reserve(max_transitions);
    EnforceMaxTransitionMemories();
}

/**
 * @brief Sets the maximum number of action memories with validation and enforcement
 * @param max_actions New maximum capacity (must be > 0)
 *
 * @algorithm Validation + assignment + immediate capacity enforcement
 * @rationale Same validation and enforcement pattern as transition memories
 * @complexity O(1) if no enforcement needed, O(k) where k = memories to remove
 */
void MemorySystem::SetMaxActionMemories(int max_actions)
{
    if (max_actions <= 0)
    {
        LogError("MemorySystem: max_actions must be greater than 0, keeping current value");
        return;
    }

    this->max_action_memories = max_actions;
    action_memories.reserve(max_actions);
    EnforceMaxActionMemories();

}

/**
 * @brief Sets the maximum number of interruption memories with validation and enforcement
 * @param max_interruptions New maximum capacity (must be > 0)
 *
 * @algorithm Validation + assignment + immediate capacity enforcement
 * @rationale Same validation and enforcement pattern as other memory types
 * @complexity O(1) if no enforcement needed, O(k) where k = memories to remove
 */
void MemorySystem::SetMaxInterruptionMemories(int max_interruptions)
{
    if (max_interruptions <= 0)
    {
        LogError("MemorySystem: max_interruptions must be greater than 0, keeping current value");
        return;
    }

    this->max_interruption_memories = max_interruptions;
    interruption_memories.reserve(max_interruptions);
    EnforceMaxInterruptionMemories();

}

// =============================================================================
// MEMORY UPDATE OPERATIONS
// =============================================================================

/**
 * @brief Records or updates a transition decision in character memory
 * @param target_node_id Unique identifier of the sequence node that was visited
 * @param current_time Simulation timestamp when the transition was taken
 * @return true if update succeeded, false if validation or creation failed
 *
 * @algorithm Remove-then-add pattern with capacity enforcement
 * @rationale
 * - Remove existing prevents duplicate memories for same node
 * - std::move avoids unnecessary copying of memory objects
 * - Exception handling ensures system stability on invalid input
 * - Capacity enforcement maintains bounded memory usage
 *
 * @complexity O(n) where n = current transition memory count (due to removal search)
 * @datastructures std::vector allows efficient push_back and maintains insertion order
 * @performance_notes
 * - Linear search for removal is acceptable given small memory capacities (5-10)
 * - Move semantics optimize memory object management
 * - Exception handling prevents crashes but logs errors for debugging
 */
bool MemorySystem::UpdateTransitionMemory(int target_node_id, int current_time)
{
    try
    {
        TransitionMemory new_memory(target_node_id, current_time);

        // Remove existing memory to prevent duplicates
        RemoveExistingTransitionMemory(target_node_id);

        // Add new memory at end (newest memories at back, oldest at front)
        // use std::move to move the created memory instead of copying it in another object
        transition_memories.push_back(std::move(new_memory));
        EnforceMaxTransitionMemories();

        return true;
    } catch (const std::exception& e)
    {
        LogError("UpdateTransitionMemory failed: " + std::string(e.what()));
        return false;
    }
}

/**
 * @brief Records or updates an action execution decision in character memory
 * @param action_id Unique identifier of the action that was performed
 * @param target_entity_id Unique identifier of the entity that was used
 * @param current_time Simulation timestamp when the action was executed
 * @return true if update succeeded, false if validation or creation failed
 *
 * @algorithm Remove-then-add pattern with compound matching and capacity enforcement
 * @rationale
 * - Compound removal (action_id + entity_id) prevents duplicate memories
 * - std::move optimization avoids unnecessary object copying
 * - Exception handling maintains system stability
 * - Capacity enforcement maintains bounded memory usage
 * @complexity O(n) where n = current action memory count (due to compound removal search)
 * @datastructures std::vector maintains insertion order (newest last, oldest first)
 */
bool MemorySystem::UpdateActionMemory(int action_id, int target_entity_id, int current_time)
{
    try
    {
        ActionMemory new_memory(action_id, target_entity_id, current_time);

        // remove the action memory if it exists already to prevent duplicates.
        RemoveExistingActionMemory(action_id, target_entity_id);

        // use std::move to move the created memory instead of copying it in another object
        action_memories.push_back(std::move(new_memory));
        EnforceMaxActionMemories();

        return true;
    } catch (const std::exception& e)
    {
        LogError("UpdateActionMemory failed: " + std::string(e.what()));
        return false;
    }
}

/**
 * @brief Records or updates an interruption context in character memory
 * @param action_id Unique identifier of the action that was interrupted
 * @param sequence_id Unique identifier of the sequence that was executing
 * @param node_id Unique identifier of the sequence node that was interrupted
 * @param entity_id Unique identifier of the entity involved (-1 if none)
 * @param current_time Simulation timestamp when the interruption occurred
 * @return true if update succeeded, false if validation or creation failed
 *
 * @algorithm Remove-then-add pattern with triple matching and capacity enforcement
 * @rationale
 * - Triple removal (action_id + sequence_id + node_id) prevents duplicate contexts
 * - Preserves complete execution context for seamless resumption
 * - Exception handling critical for interruption scenarios
 * - Capacity enforcement prevents unbounded context accumulation
 * @complexity O(n) where n = current interruption memory count (due to triple removal search)
 * @datastructures std::vector maintains insertion order for FIFO capacity management
 */
bool MemorySystem::UpdateInterruptionMemory(int action_id, int sequence_id, int node_id, int entity_id,
    int current_time)
{
    try
    {
        InterruptionMemory new_memory(action_id, sequence_id, node_id, entity_id, current_time);

        // remove interruption memory if it exists already to prevent duplicates.
        RemoveInterruptionMemory(action_id, sequence_id, node_id);

        // use std::move to move the created memory instead of copying it in another object
        interruption_memories.push_back(std::move(new_memory));
        EnforceMaxInterruptionMemories();

        return true;

    } catch (const std::exception& e)
    {
        LogError("UpdateInterruptionMemory failed: " + std::string(e.what()));
        return false;
    }
}

// =============================================================================
// MEMORY SEARCH OPERATIONS
// =============================================================================

/**
 * @brief Searches for a transition memory matching the specified node ID
 * @param target_node_id Node identifier to search for
 * @return Pointer to matching memory or nullptr if not found
 *
 * @algorithm Linear search using std::find_if with lambda predicate
 * @rationale
 * - Linear search necessary since memories aren't sorted by node_id
 * - Lambda function provides clean, readable matching logic
 * - const return pointer prevents external modification of memory state
 * - nullptr return allows safe null-checking by callers
 * @complexity O(n) where n = transition_memories.size()
 * @datastructures std::vector allows efficient iteration, small size makes linear search acceptable
 * @performance_notes Could optimize with hash map for large memory capacities (>50)
 */
const TransitionMemory * MemorySystem::FindTransitionMemory(int target_node_id) const
{
    auto iterator = std::find_if(transition_memories.begin(), transition_memories.end(),
        [target_node_id](const TransitionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(target_node_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  transition_memories.end()) ? &(*iterator) : nullptr;
}

/**
 * @brief Searches for an action memory matching the specified action and entity
 * @param action_id Action identifier to search for
 * @param target_entity_id Entity identifier to search for
 * @return Pointer to matching memory or nullptr if not found
 *
 * @algorithm Linear search using std::find_if with compound lambda predicate
 * @rationale
 * - Compound matching (action_id + entity_id) enables fine-grained memory lookup
 * - Lambda captures both parameters for clean compound comparison
 * - const return pointer maintains memory integrity
 * @complexity O(n) where n = action_memories.size()
 * @datastructures std::vector iteration with compound matching predicate
 */
const ActionMemory * MemorySystem::FindActionMemory(int action_id, int target_entity_id) const
{
    auto iterator = std::find_if(action_memories.begin(), action_memories.end(),
        [action_id, target_entity_id](const ActionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, target_entity_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  action_memories.end()) ? &(*iterator) : nullptr;
}

/**
 * @brief Searches for an interruption memory matching the specified execution context
 * @param action_id Action identifier to search for
 * @param sequence_id Sequence identifier to search for
 * @param node_id Sequence node identifier to search for
 * @return Pointer to matching memory or nullptr if not found
 *
 * @algorithm Linear search using std::find_if with triple lambda predicate
 * @rationale
 * - Triple matching (action_id + sequence_id + node_id) ensures precise context identification
 * - Lambda captures all three parameters for complete context comparison
 * - Entity_id not included in search (preserved for resumption but not identity)
 * @complexity O(n) where n = interruption_memories.size()
 * @datastructures std::vector iteration with triple matching predicate
 */
const InterruptionMemory * MemorySystem::FindInterruptionMemory(int action_id, int sequence_id, int node_id) const
{
    auto iterator = std::find_if(interruption_memories.begin(), interruption_memories.end(),
        [action_id, sequence_id, node_id](const InterruptionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, sequence_id, node_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  interruption_memories.end()) ? &(*iterator) : nullptr;

}

// =============================================================================
// BEHAVIORAL SELECTION
// =============================================================================

/**
 * @brief Selects the least recently visited node from available options
 * @param node_ids Vector of node identifiers that are currently valid choices
 * @return Node ID that should be selected for behavioral variety, or -1 on error
 *
 * @complexity O(n*m) where n = node_ids.size(), m = transition_memories.size()
 * @datastructures
 * - std::vector for input options (allows efficient iteration)
 * - Internal vectors for collecting unused/oldest nodes
 * @performance_notes
 * - Complexity acceptable for small input sizes (typical: 2-5 nodes)
 * - Could optimize with hash maps for larger node sets (>20 nodes)
 */
int MemorySystem::GetLeastRecentlyVisitedNode(const std::vector<int> &node_ids) const
{
    if (node_ids.empty())
    {
        LogError("GetLeastRecentlyVisitedNode: There are no node_ids to search through");
        return -1;
    }

    // Try to get an unused node first
    std::vector<int> unused {FindUnusedTransitionNodes(node_ids)};
    if (!unused.empty())
    {
        // Random selection among unused nodes prevents deterministic patterns
        return SelectRandomFromVector(unused);
    }

    // If all are used find the oldest one
    return FindOldestTransitionNode(node_ids);
}

/**
 * @brief Selects the least recently used entity for a specific action
 * @param action_id Unique identifier of the action being performed
 * @param entity_ids Vector of entity identifiers that support the action
 * @return Entity ID that should be selected for variety, or -1 on error
 *
 * @algorithm Action-specific two-stage selection with compound memory matching
 * @rationale
 * - Same two-stage approach as transition selection (unused then least recently used)
 * - Action-specific memory lookup enables entity variety per action type
 * - Allows same entity for different actions, different entities for same action
 * @complexity O(n*m) where n = entity_ids.size(), m = action_memories.size()
 * @datastructures
*  - std::vector for input options (allows efficient iteration)
 * - Internal vectors for collecting unused/oldest entities
 */
int MemorySystem::GetLeastRecentlyUsedEntityForAction(int action_id, const std::vector<int> &entity_ids) const
{
    if (entity_ids.empty())
    {
        LogError("GetLeastRecentlyUsedEntityForAction: There are no entity_ids to search through");
        return -1;
    }

    // Try to get an unused entity first
    std::vector<int> unused {FindUnusedActionEntities(action_id, entity_ids)};
    if (!unused.empty())
    {
        // Random selection among unused entities prevents deterministic patterns
        return SelectRandomFromVector(unused);
    }

    // If all are used find the oldest one
    return FindOldestActionEntity(action_id, entity_ids);
}

// =============================================================================
// MEMORY CLEANUP OPERATIONS
// =============================================================================

/**
 * @brief Removes all interruption memories associated with a specific sequence
 * @param sequence_id Unique identifier of the sequence whose memories should be cleared
 *
 * @algorithm Reverse iteration with conditional removal
 * @rationale
 * - Reverse iteration prevents iterator invalidation during removal
 * - Sequence-based cleanup essential for handling sequence failures
 * - Maintains memory consistency when sequences are abandoned or reset
 * - Used when fallback sequences are activated due to failures
 * @complexity O(n) where n = interruption_memories.size()
 * @datastructures std::vector::erase() with reverse indexing for safe removal
 * @performance_notes Reverse iteration more efficient than forward iteration with erase()
 */
void MemorySystem::ClearSequenceInterruptionMemories(int sequence_id)
{
    // Iterate backwards through the vector to remove the elements safely.
    for (int i = static_cast<int>(interruption_memories.size()) - 1; i >= 0; i--)
    {
        if (interruption_memories[i].GetInterruptedSequenceId() == sequence_id)
        {
            interruption_memories.erase(interruption_memories.begin() + i);
        }
    }
}

/**
 * @brief Removes a specific interruption memory matching the execution context
 * @param action_id Action identifier of the memory to remove
 * @param sequence_id Sequence identifier of the memory to remove
 * @param node_id Sequence node identifier of the memory to remove
 * @return true if memory was found and removed, false if not found
 *
 * @algorithm Linear search with conditional removal
 * @rationale
 * - Triple matching ensures precise memory identification for removal
 * - Used when interruption contexts are successfully resumed
 * - Boolean return indicates whether cleanup was necessary
 * - Prevents memory leaks from abandoned interruption contexts
 * @complexity O(n) where n = interruption_memories.size()
 * @datastructures std::find_if + std::vector::erase() for targeted removal
 */
bool MemorySystem::RemoveInterruptionMemory(int action_id, int sequence_id, int node_id)
{
    // Search for existing memory with same action_id AND sequence_id AND node_id
    auto iterator = std::find_if(interruption_memories.begin(), interruption_memories.end(),
        [action_id, sequence_id, node_id](const InterruptionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, sequence_id, node_id); // return true if it matches
        });

    // Remove existing memory if found
    if (iterator != interruption_memories.end()) {
        interruption_memories.erase(iterator);
        return true;
    }

    return false;
}

/**
 * @brief Clears all memories from all memory collections
 *
 * @algorithm Direct vector clearing for all memory types
 * @rationale
 * - Complete reset functionality for character initialization or debugging
 * - Ensures clean state without memory leaks or stale references
 * - Used during character creation or when resetting behavioral state
 * @complexity O(1) - std::vector::clear() is constant time
 * @datastructures Calls clear() on all std::vector instances
 */
void MemorySystem::ClearAllMemories()
{
    transition_memories.clear();
    action_memories.clear();
    interruption_memories.clear();
}

// =============================================================================
// UTILITY HELPERS
// =============================================================================

/**
 * @brief Logs error messages to standard error output
 * @param message Error message to log
 *
 * @algorithm Direct output to std::cerr with formatted prefix
 * @rationale
 * - Centralized error logging for consistent message formatting
 * - std::cerr ensures error visibility even when stdout is redirected
 * - Formatted prefix enables easy filtering of memory system errors
 * - Non-throwing design maintains system stability during errors
 * @complexity O(1) - simple string output
 * @datastructures Uses std::string for message handling
 */
void MemorySystem::LogError(const std::string &message) const
{
    std::cerr << "[MemorySystem Error]: " << message << std::endl;
}

/**
 * @brief Selects a random element from a vector of integers
 * @param options Vector of integer options to choose from
 * @return Randomly selected integer from the vector, or -1 if empty
 *
 * @algorithm Modulo-based random selection using rand()
 * @rationale
 * - Random selection prevents deterministic behavioral patterns
 * - Modulo operation provides uniform distribution across options
 * - Error handling for empty vectors prevents crashes
 * - Simple rand() acceptable for behavioral variety (not cryptographic security)
 * @complexity O(1) - single random number generation and modulo operation
 * @datastructures std::vector random access by index
 * @performance_notes Could upgrade to std::random_device for better randomness
 */
int MemorySystem::SelectRandomFromVector(const std::vector<int> &options) const
{
    if (options.empty())
    {
        LogError("SelectRandomFromVector: There are no options to search through");
        return -1;
    }

    return options[rand() % options.size()];
}

// =============================================================================
// CAPACITY MANAGEMENT
// =============================================================================

/**
 * @brief Enforces maximum transition memory capacity by removing the oldest entries
 *
 * @algorithm Conditional oldest-first removal using front-of-vector deletion
 * @rationale
 * - FIFO removal maintains recency-based ordering (oldest memories removed first)
 * - Front deletion leverages vector ordering (newest at back, oldest at front)
 * - Only removes when necessary (count > max) for efficiency
 * - Maintains bounded memory usage for performance scalability
 * @complexity O(n) due to std::vector::erase() from beginning
 * @datastructures std::vector front removal shifts all remaining elements
 * @optimization_notes Could use std::deque for O(1) front removal if capacity limits are large
 */
void MemorySystem::EnforceMaxTransitionMemories()
{
    auto excess_memories_count = transition_memories.size() > max_transition_memories ?
        static_cast<int>(transition_memories.size() - max_transition_memories) :
        0;

    if ( excess_memories_count > 0)
    {
        transition_memories.erase(transition_memories.begin(), transition_memories.begin() + excess_memories_count);
    }
}

/**
 * @brief Enforces maximum action memory capacity by removing the oldest entries
 *
 * @algorithm Same FIFO enforcement pattern as transition memories
 * @rationale Same bounded memory management strategy
 * @complexity O(n) due to front vector deletion
 */
void MemorySystem::EnforceMaxActionMemories()
{
    auto excess_memories_count = action_memories.size() > max_action_memories ?
        static_cast<int>(action_memories.size() - max_action_memories) :
        0;

    if ( excess_memories_count > 0)
    {
        action_memories.erase(action_memories.begin(), action_memories.begin() + excess_memories_count);
    }
}

/**
 * @brief Enforces maximum interruption memory capacity by removing the oldest entries
 *
 * @algorithm Same FIFO enforcement pattern as other memory types
 * @rationale Same bounded memory management strategy
 * @complexity O(n) due to front vector deletion
 */
void MemorySystem::EnforceMaxInterruptionMemories()
{
    auto excess_memories_count = interruption_memories.size() > max_interruption_memories ?
        static_cast<int>(interruption_memories.size() - max_interruption_memories) :
        0;

    if ( excess_memories_count > 0)
    {
        interruption_memories.erase(interruption_memories.begin(), interruption_memories.begin() + excess_memories_count);
    }
}

// =============================================================================
// MEMORY REMOVAL HELPERS
// =============================================================================

/**
 * @brief Removes existing transition memory matching the specified node ID
 * @param target_node_id Node identifier of the memory to remove
 *
 * @algorithm Linear search with conditional removal
 * @rationale
 * - Used by UpdateTransitionMemory to implement update-not-duplicate semantics
 * - Silent failure (no error if not found) since absence is valid state
 * - Maintains memory uniqueness constraint for transition decisions
 * @complexity O(n) where n = transition_memories.size()
 * @datastructures std::find_if + conditional std::vector::erase()
 */
void MemorySystem::RemoveExistingTransitionMemory(int target_node_id)
{
    // Search for existing memory with same target_node_id
    auto iterator = std::find_if(transition_memories.begin(), transition_memories.end(),
        [target_node_id](const TransitionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(target_node_id); // return true if it matches
        });

    // Remove existing memory if found
    if (iterator != transition_memories.end()) {
        transition_memories.erase(iterator);
    }
}

/**
 * @brief Removes existing action memory matching the specified action and entity
 * @param action_id Action identifier of the memory to remove
 * @param target_entity_id Entity identifier of the memory to remove
 *
 * @algorithm Linear search with compound matching and conditional removal
 * @rationale
 * - Used by UpdateActionMemory to implement update-not-duplicate semantics
 * - Compound matching ensures precise memory identification
 * - Silent failure maintains robustness during updates
 * @complexity O(n) where n = action_memories.size()
 * @datastructures std::find_if with compound lambda + conditional std::vector::erase()
 */
void MemorySystem::RemoveExistingActionMemory(int action_id, int target_entity_id)
{
    // Search for existing memory with same target_node_id
    auto iterator = std::find_if(action_memories.begin(), action_memories.end(),
        [action_id, target_entity_id](const ActionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, target_entity_id); // return true if it matches
        });

    // Remove existing memory if found
    if (iterator != action_memories.end()) {
        action_memories.erase(iterator);
    }
}

// =============================================================================
// RECENCY SELECTION HELPERS
// =============================================================================

/**
 * @brief Finds the oldest transition node among the given options
 * @param node_ids Vector of node identifiers to search through
 * @return Node ID with the oldest timestamp, or -1 on error
 *
 * @algorithm Linear search with timestamp comparison and tie handling
 * @rationale
 * - Linear search necessary since memories aren't sorted by timestamp
 * - Tie handling prevents deterministic selection when timestamps match
 * - Multiple nodes with same timestamp get random selection
 * - Error logging helps debug missing memory scenarios
 * @complexity O(n*m) where n = node_ids.size(), m = transition_memories.size()
 * @datastructures
 * - std::vector for collecting tied results
 * - INT_MAX as initial comparison value
 * @performance_notes
 * - Could optimize with timestamp indexing for larger memory sizes
 * - Current approach prioritizes simplicity over micro-optimizations
 * - Random tie-breaking essential for behavioral variety
 */
int MemorySystem::FindOldestTransitionNode(const std::vector<int> &node_ids) const
{
    if (node_ids.empty())
    {
        LogError("FindOldestTransitionNode: There are no node_ids to search through");
        return -1;
    }

    std::vector<int> oldest_nodes;
    oldest_nodes.reserve(node_ids.size());
    int oldest_time = INT_MAX;

    // Search for memories and track oldest timestamp(s)
    for (int node_id : node_ids)
    {
        const TransitionMemory* memory = FindTransitionMemory(node_id);
        if (memory == nullptr)
        {
            LogError("FindOldestTransitionNode: Expected memory not found for node " + std::to_string(node_id));
            continue;
        }

        int current_time {memory->GetLastUsedTime()};

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.emplace_back(node_id);
        } // If it there is a tie, we just add it to the existing oldest_nodes vector.
        else if (current_time == oldest_time)
        {
            oldest_nodes.emplace_back(node_id);
        }
    }

    if (oldest_nodes.empty())
    {
        LogError("FindOldestTransitionNode: No valid memories found");
        return -1;
    }

    // Random selection among tied options prevents deterministic patterns
    return SelectRandomFromVector(oldest_nodes);
}

/**
 * @brief Finds the oldest action entity for a specific action among given options
 * @param action_id Action identifier to search for
 * @param entity_ids Vector of entity identifiers to compare
 * @return Entity ID with the oldest timestamp for this action, or -1 on error
 *
 * @algorithm Linear search with action-specific timestamp comparison and tie handling
 * @rationale
 * - Action-specific memory lookup enables per-action entity variety
 * - Same timestamp comparison and tie-breaking logic as transition nodes
 * - Error logging helps debug missing memory scenarios for action-entity combinations
 * @complexity O(n*m) where n = entity_ids.size(), m = action_memories.size()
 * @datastructures std::vector for collecting tied results + action-specific memory lookup
 * @performance_notes Compound matching adds overhead but enables richer behavioral variety
 */
int MemorySystem::FindOldestActionEntity(int action_id, const std::vector<int> &entity_ids) const
{
    if (entity_ids.empty())
    {
        LogError("FindOldestActionEntity: There are no entity_ids to search through");
        return -1;
    }

    std::vector<int> oldest_nodes;
    oldest_nodes.reserve(entity_ids.size());
    int oldest_time = INT_MAX;

    // Search for action-specific memories and track oldest timestamp(s)
    for (int entity_id : entity_ids)
    {
        const ActionMemory* memory = FindActionMemory(action_id, entity_id);
        if (memory == nullptr)
        {
            LogError("FindOldestActionEntity: Expected memory not found for action " + std::to_string(action_id) +
                " and entity " + std::to_string(entity_id));

            continue;
        }

        int current_time = memory->GetLastUsedTime();

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.emplace_back(entity_id);
        }
        else if (current_time == oldest_time)
        {
            oldest_nodes.emplace_back(entity_id);
        }
    }

    if (oldest_nodes.empty())
    {
        LogError("FindOldestActionEntity: No valid memories found");
        return -1;
    }

    // Random selection among tied options prevents deterministic patterns
    return SelectRandomFromVector(oldest_nodes);
}

/**
 * @brief Finds transition nodes that have never been visited (no memory exists)
 * @param node_ids Vector of node identifiers to check
 * @return Vector of node IDs that have no corresponding transition memory
 *
 * @algorithm Linear search with negative matching (collect nodes WITHOUT memories)
 * @rationale
 * - Enables exploration priority (unused nodes selected before LRU nodes)
 * - Null memory check identifies truly unused options
 * - Encourages behavioral variety by prioritizing new experiences
 * @complexity O(n*m) where n = node_ids.size(), m = transition_memories.size()
 * @datastructures std::vector accumulation of nodes without corresponding memories
 * @performance_notes Could optimize with memory indexing for large node sets
 */
std::vector<int> MemorySystem::FindUnusedTransitionNodes(const std::vector<int> &node_ids) const
{
    std::vector<int> unused_nodes;
    for (int node_id : node_ids)
    {
        const TransitionMemory* memory = FindTransitionMemory(node_id);
        if (memory == nullptr)
        {
            unused_nodes.emplace_back(node_id);
        }
    }

    return unused_nodes;
}

/**
 * @brief Finds entities that have never been used for a specific action
 * @param action_id Action identifier to check for
 * @param entity_ids Vector of entity identifiers to check
 * @return Vector of entity IDs that have no corresponding action memory for this action
 *
 * @algorithm Linear search with action-specific negative matching
 * @rationale
 * - Enables exploration priority for action-entity combinations
 * - Action-specific checking allows same entity for different actions
 * - Prioritizes entity variety within each action type
 * @complexity O(n*m) where n = entity_ids.size(), m = action_memories.size()
 * @datastructures std::vector accumulation with action-specific memory checking
 */
std::vector<int> MemorySystem::FindUnusedActionEntities(int action_id, const std::vector<int> &entity_ids) const
{
    std::vector<int> unused_entities;
    for (int entity_id : entity_ids)
    {
        const ActionMemory* memory = FindActionMemory(action_id, entity_id);
        if (memory == nullptr)
        {
            unused_entities.emplace_back(entity_id);
        }
    }

    return unused_entities;
}

// =============================================================================
// DIAGNOSTIC & MONITORING
// =============================================================================

size_t MemorySystem::GetTransitionMemoryCount() const
{
    return transition_memories.size();
}

size_t MemorySystem::GetActionMemoryCount() const
{
    return action_memories.size();
}

size_t MemorySystem::GetInterruptionMemoryCount() const
{
    return interruption_memories.size();
}
