/*
* ActionMemory.h
 *
 * Memory storage for action execution decisions in ambient character behavior.
 * Tracks which actions have been performed on specific entities to create
 * natural variety in entity selection and prevent repetitive interactions.
 *
 * Author: Eric Buitrón López
 * Created: 8/11/2025
 */

#pragma once
#include "IMemory.h"

class ActionMemory : public IMemory {
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    int action_id;
    int target_entity_id;

public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    ActionMemory(int action_id, int target_entity_id, int last_used_time);

    // =============================================================================
    // MEMORY MATCHING (Framework Core Functionality)
    // =============================================================================

    bool MatchesMemory(const IMemory& other) const override;
    bool MatchesMemory(int other_action_id, int other_target_entity_id) const;

    // =============================================================================
    // DATA ACCESS
    // =============================================================================

    int GetActionId() const;
    int GetTargetEntityId() const;

};