#include "BehavioralEntity.h"

#include <algorithm>

#include "services/composition/ServiceBuilder.h"

using namespace AmbientCharacterBehavior;

void BehavioralEntity::SetMainSequence(const std::shared_ptr<Sequence> &new_sequence)
{
    if (new_sequence && new_sequence != main_sequence)
    {
        main_sequence = new_sequence;
    }
}

void BehavioralEntity::AddFallbackSequence(const std::shared_ptr<Sequence> &new_sequence)
{
    if (new_sequence && !HasFallbackSequence(new_sequence->GetSequenceId()))
    {
        fallback_sequences.emplace_back(new_sequence);
    }
}

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

void BehavioralEntity::AddInterruptionHandler(int32_t interruption_id, const std::shared_ptr<Sequence> &handler)
{
    if (handler)
    {
        interruption_handlers.insert_or_assign(interruption_id, handler);
    }
}

std::shared_ptr<Sequence> BehavioralEntity::FindInterruptionHandler(int32_t interruption_id) const
{
    auto iterator = interruption_handlers.find(interruption_id);
    if (iterator == interruption_handlers.end())
    {
        //TODO: replace with logger
        //logger.LogWarning("FrameworkEntity[" + std::to_string(entity_id) + "]: Interruption id: " +
        //    std::to_string(interruption_id) + " not found", "BehavioralEntity");

        return nullptr;
    }

    return iterator->second;
}
