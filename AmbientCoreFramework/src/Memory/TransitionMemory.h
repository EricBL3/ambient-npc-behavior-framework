/*
* TransitionMemory.h
 *
 * Memory storage for sequence transition decisions in ambient character behavior.
 * Tracks which sequence nodes have been visited to prevent repetitive behavioral loops.
 *
 * Author: Eric Buitrón López
 * Created: 8/11/2025
 */

#pragma once
#include "IMemory.h"

class TransitionMemory : public IMemory
{
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================
    int target_node_id;

public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    TransitionMemory(int node_id, int time);

    // =============================================================================
    // MEMORY MATCHING
    // =============================================================================

    bool MatchesMemory(const IMemory& other) const override;
    bool MatchesMemory(int other_node_id) const;

    // =============================================================================
    // DATA ACCESS
    // =============================================================================
    int GetTargetNodeId() const;
};
