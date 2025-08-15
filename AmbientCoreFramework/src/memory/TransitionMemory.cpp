/**
 * @file TransitionMemory.cpp
 * @brief Implementation of transition decision tracking for ambient character behavioral variety
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
 * Stores which sequence nodes characters have visited to enable recency-based selection that prevents repetitive
 * behavioral patterns in ambient NPCs.
 */

#include "TransitionMemory.h"

// =============================================================================
// CONSTRUCTION
// =============================================================================

/**
 * @brief Constructs transition memory for a specific node visit
 * @param node_id Unique identifier of the sequence node that was visited
 * @param time Simulation timestamp when the transition was taken
 * @throws std::invalid_argument if node_id < 0 or time < 0
 *
 * @algorithm Simple validation + member initialization
 * @rationale Validation ensures data integrity. Delegates time validity to base class
 * @complexity O(1)  - simple assignment operations
 */
TransitionMemory::TransitionMemory(int node_id, int time) : IMemory(time)
{
    if(node_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: node_id cannot be negative, got " + std::to_string(node_id));
    }
    
    target_node_id = node_id;
    // Note: last_used_time is set by the IMemory constructor
}

// =============================================================================
// MEMORY MATCHING
// =============================================================================

/**
 * @brief Convenience method to check if this memory matches a specific node ID
 * @param other_node_id Node identifier to compare against
 * @return true if this memory's target_node_id matches other_node_id
 * @note More efficient than the IMemory version when you already know you're working with node IDs
 *
 * @algorithm Direct integer comparison
 * @complexity O(1) - single integer comparison
 */
bool TransitionMemory::MatchesMemory(int other_node_id) const
{
    return this->target_node_id == other_node_id;
}

/**
 * @brief Checks if this memory matches another memory instance polymorphically
 *
 * Implements the IMemory interface by safely checking if the other memory
 * is also a TransitionMemory with the same target_node_id. Uses dynamic_cast
 * for type safety in polymorphic scenarios.
 *
 * @param other Memory instance to compare against
 * @return true if other is TransitionMemory with same target_node_id
 *
 * @algorithm Two-stage matching: type check + value comparison
 *
 * @rationale
 * - dynamic_cast provides type safety for polymorphic collections
 * - Delegates to optimized node-specific comparison after type verification
 * - Returns false for incompatible types rather than throwing exceptions
 *
 * @complexity O(1) - dynamic_cast + single integer comparison
 * @datastructures Uses dynamic_cast for safe polymorphic type checking
 * @performance_notes
 * - dynamic_cast has slight overhead but acceptable for decision-making frequency
 * - Alternative approaches (type enums) would require more complex maintenance
 * - Current approach prioritizes type safety over micro-optimizations
 */
bool TransitionMemory::MatchesMemory(const IMemory& other) const
{
    // Safe type conversion that returns nullptr if other is not a TransitionMemory
    const auto other_transition_memory = dynamic_cast<const TransitionMemory*>(&other);
    if(!other_transition_memory)
    {
        return false;
    }

    return MatchesMemory(other_transition_memory->target_node_id);
}

// =============================================================================
// DATA ACCESS
// =============================================================================

/**
 * @brief Gets the sequence node ID that this memory represents
 * @return Unique identifier of the sequence node that was visited
 *
 * @algorithm Direct member access
 * @complexity O(1) - simple member variable access
 */
int TransitionMemory::GetTargetNodeId() const
{
    return this->target_node_id;
}


