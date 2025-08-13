//
// Created by Eric on 8/11/2025.
//

#pragma once
#include "IMemory.h"

class ActionMemory : public IMemory {
private:
    int action_id;
    int target_entity_id;

public:
    ActionMemory(int action_id, int target_entity_id, int last_used_time);
    bool MatchesMemory(const IMemory& other) const override;
    bool MatchesMemory(int other_action_id, int other_target_entity_id) const;
    int GetActionId() const;
    int GetTargetEntityId() const;

};