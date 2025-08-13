//
// Created by Eric on 8/11/2025.
//

#pragma once
#include "IMemory.h"

class TransitionMemory : public IMemory
{
private:
    int target_node_id;

public:
    TransitionMemory(int node_id, int time);
    bool MatchesMemory(const IMemory& other) const override;
    bool MatchesMemory(int other_node_id) const;
    int GetTargetNodeId() const;
};
