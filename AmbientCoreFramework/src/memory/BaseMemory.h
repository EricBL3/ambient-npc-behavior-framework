/**
 * @file BaseMemory.h
 * @brief Base class for all memory types in the ambient character behavior framework
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
 * @defgroup memory_group Memory Group
 * @brief Character decision history tracking to prevent repetitive behaviors
 *
 * The Memory Group implements the core variety-generation mechanism of the framework.
 * By tracking recent character decisions across different contexts, it enables characters
 * to avoid repetitive patterns without requiring complex scripting or utility calculations.
 *
 * @{
 */

#pragma once
#include <stdexcept>
#include <string>

namespace AmbientCharacterBehavior {
/**
 * @brief Abstract base class for all memory types in the behavior framework.
 *
 * BaseMemory provides the fundamental abstraction that allows the memory system
 * to manage different types of character memories (transitions, actions,
 * interruptions) through a uniform interface while preserving their specific
 * matching and comparison behaviors.
 *
 * **Performance**
 * - Construction: O(1)
 * - Time comparison: O(1)
 * - Memory matching: O(1) - depends on the derived class implementation
 *
 * @note All concrete implementations must handle negative time validation and implement specific matching logic.
 *
 * @see TransitionMemory, ActionMemory, InterruptionMemory, MemorySystem
 */
class BaseMemory
{
protected:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    /**
     * @brief Simulation timestamp when this memory was last updated
     *
     * Used for recency based comparisons to implement least recently used selection.
     * Must be non-negative and typically represents game time or frame count.
     *
     * @invariant last_used_time >= 0
     */
    int last_used_time;
public:
    // =============================================================================
    // CONSTRUCTION & DESTRUCTION
    // =============================================================================

    /**
     * @brief Constructs memory with the specified timestamp
     *
     * @param time Simulation time when this memory should be considered created
     *
     * @throws std::invalid_argument if time < 0
     *
     * @pre time >= 0
     * @post last_used_time == time
     */
    explicit BaseMemory(const int time) : last_used_time(time) {
        if(time < 0)
        {
            throw std::invalid_argument("BaseMemory: time cannot be negative, got " + std::to_string(time));
        }
    }

    /**
     * @brief Virtual destructor for proper polymorphic cleanup
     *
     * Ensures derived classes are properly destroyed when accessed through BaseMemory pointers.
     */
    virtual ~BaseMemory() = default;

    // =============================================================================
    // MEMORY INTERFACE
    // =============================================================================

    /**
     * @brief Determines if this memory represents the same decision as another
     *
     * Pure virtual method that must be implemented by each memory class implementation to define their specific matching
     * criteria. Used by the memory system to detect when a decision is being repeated vs. when it's a new decision.
     *
     * **Implementation Requirements:**
     * - Must be symmetric: a.MatchesMemory(b) == b.MatchesMemory(a)
     * - Must be reflexive: a.MatchesMemory(a) == true
     * - Should only compare relevant identifying fields, not timestamps
     *
     * @param other Memory to compare against
     * @return true if this memory represents the same decision context
     *
     * @note Performance critical - called during every character decision cycle
     */
    [[nodiscard]]
    virtual bool MatchesMemory(const BaseMemory& other) const = 0;

    // =============================================================================
    // TIME-BASED OPERATIONS
    // =============================================================================

    /**
     * @brief Gets the timestamp when this memory was last updated
     *
     * @return Simulation time value used for recency comparisons
     *
     * @post return value >= 0
     */
    int GetLastUsedTime() const { return last_used_time; }

    /**
     * @brief Compares recency with another memory least recently used selection
     *
     * Enables the memory system to  identify which memories are older and should be prioritized for selection or
     * removal.
     *
     * @param other Memory to compare recency against
     * @return true if this memory is older (has a smaller timestamp)
     *
     * @note Does not check if memories are of the same type or represent the same decision. It only compares timestamps.
     *
     * @see GetLastUsedTime()
     */
    virtual bool IsOlderThan(BaseMemory& other) const {
        return GetLastUsedTime() < other.GetLastUsedTime();
    }
};

}

/** @} */ // End of memory_group