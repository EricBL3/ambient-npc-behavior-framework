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
        logger.LogWarning("Sequence with id: " + std::to_string(sequence_id) + " is not in the fallback sequences",
         "BehavioralEntity");
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
        logger.LogWarning("FrameworkEntity[" + std::to_string(entity_id) + "]: Interruption id: " +
            std::to_string(interruption_id) + " not found", "BehavioralEntity");

        return nullptr;
    }

    return iterator->second;
}

void BehavioralEntity::ExecuteCurrentSequence()
{
    logger.LogInfo("Executing current sequence for entity: " + std::to_string(entity_id),
        "BehavioralEntity");
}

void BehavioralEntity::ProcessInterruption(int32_t interruption_id)
{
    logger.LogInfo("Processing interruption with id: " + std::to_string(interruption_id),
        "BehavioralEntity");
}

void BehavioralEntity::CompleteAction(int32_t action_id, int64_t action_token)
{
    if (CompletedCurrentAction(action_id, action_token))
    {
        logger.LogWarning("entity with id: " + std::to_string(entity_id) + " has completed action with id: " +
            std::to_string(action_id) + " and token: " + std::to_string(action_token) ,"BehavioralEntity");
    }
}

bool BehavioralEntity::CompletedCurrentAction(int32_t action_id, int64_t action_token) const
{
    if (action_token != current_action_token || action_id != current_action_id)
    {
        logger.LogWarning("The completed action with id: " + std::to_string(action_id) + " and token: " +
            std::to_string(action_token) + " is not the same as the currently executing action for entity with id: " +
            std::to_string(entity_id),"BehavioralEntity");

        return false;
    }

    return true;
}
