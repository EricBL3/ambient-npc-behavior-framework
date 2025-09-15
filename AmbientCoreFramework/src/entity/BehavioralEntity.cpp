#include "BehavioralEntity.h"

#include <algorithm>

using namespace AmbientCharacterBehavior;

bool BehavioralEntity::HasFallbackSequence(int32_t sequence_id) const
{
    return std::any_of(fallback_sequences.begin(), fallback_sequences.end(),
                       [sequence_id](const std::shared_ptr<Sequence>& seq) {
                           return seq && seq->GetSequenceId() == sequence_id;
                       });
}

std::shared_ptr<Sequence> BehavioralEntity::GetFallbackSequenceById(int32_t sequence_id) const
{
    auto iterator = std::find_if(fallback_sequences.begin(), fallback_sequences.end(),
                           [sequence_id](const std::shared_ptr<Sequence>& seq) {
                               return seq && seq->GetSequenceId() == sequence_id;
                           });

    if (iterator == fallback_sequences.end())
    {
        // TODO: replace with logger
        // logger.LogWarning("Sequence with id: " + std::to_string(sequence_id) + " is not in the fallback sequences",
        // "BehavioralEntity");
        return nullptr;
    }

    return *iterator;
}