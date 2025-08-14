/*
* InterruptionMemory.h
 *
 * Memory storage for interrupted action context in ambient character behavior.
 * Enables resumption of actions after external interruptions by preserving the execution state.
 *
 * Author: Eric Buitrón López
 * Created: 8/12/2025
 */


#include "IMemory.h"

class InterruptionMemory : public IMemory {
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    int interrupted_action_id;
    int interrupted_sequence_id;
    int interrupted_sequence_node_id;
    int interrupted_target_entity_id;
public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    InterruptionMemory(
        int interrupted_action_id,
        int interrupted_sequence_id,
        int interrupted_sequence_node_id,
        int interrupted_target_entity_id,
        int time
    );

    // =============================================================================
    // MEMORY MATCHING
    // =============================================================================

    bool MatchesMemory(const IMemory& other) const override;
    bool MatchesMemory(int other_action_id, int other_sequence_id, int other_sequence_node_id) const;

    // =============================================================================
    // CONTEXT DATA ACCESS
    // =============================================================================
    int GetInterruptedActionId() const;
    int GetInterruptedSequenceId() const;
    int GetInterruptedSequenceNodeId() const;
    int GetInterruptedTargetEntityId() const;
};
