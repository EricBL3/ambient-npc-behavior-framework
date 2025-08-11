#pragma once

#include "IMemory.h"

class TransitionMemory : public IMemory
{
private:
    int target_node_id;
    int last_used_time;
public:
    TransitionMemory(int node_id, int time);

    bool MatchesMemory(const IMemory& other) const override;
    int GetLastUsedTime() const override;
    bool IsOlderThan(const IMemory& other) const override;

    int GetTargetNodeId() const;
};
