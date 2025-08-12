//
// Created by Eric Buitron on 2025-08-12.
//


#include "IMemory.h"

class InterruptionMemory : public IMemory {
private:
    int interrupted_action_id;
    int interrupted_sequence_id;
    int interrupted_sequence_node_id;
    int interrupted_target_entity_id;
public:
    InterruptionMemory(
        int interrupted_action_id,
        int interrupted_sequence_id,
        int interrupted_sequence_node_id,
        int interrupted_target_entity_id
    );

    bool MatchesMemory(const IMemory& other) const override;
    int GetInterruptedActionId() const;
    int GetInterruptedSequenceId() const;
    int GetInterruptedSequenceNodeId() const;
    int GetInterruptedTargetEntityId() const;
};
