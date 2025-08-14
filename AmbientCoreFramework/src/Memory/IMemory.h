/*
* IMemory.h
 *
 * Base interface for all memory types in the ambient character behavior framework.
 * Provides common functionality for time tracking and polymorphic memory operations
 * that enable the memory system to manage different memory types uniformly.
 *
 * Author: Eric Buitrón López
 * Created: 8/11/2025
 */

#pragma once
#include <stdexcept>
#include <string>

class IMemory
{
protected:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    int last_used_time;
public:
    // =============================================================================
    // CONSTRUCTION & DESTRUCTION
    // =============================================================================

    IMemory(int time) {
        if(time < 0)
        {
            throw std::invalid_argument("TransitionMemory: time cannot be negative, got " + std::to_string(time));
        }
        last_used_time = time;
    }

    virtual ~IMemory() = default;

    // =============================================================================
    // MEMORY INTERFACE
    // =============================================================================

    virtual bool MatchesMemory(const IMemory& other) const = 0;

    // =============================================================================
    // TIME-BASED OPERATIONS
    // =============================================================================
    int GetLastUsedTime() const { return last_used_time; }
    virtual bool IsOlderThan(const IMemory& other) const {
        return last_used_time < other.last_used_time;
    }
};