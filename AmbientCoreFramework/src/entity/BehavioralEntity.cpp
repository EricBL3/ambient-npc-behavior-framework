#include "BehavioralEntity.h"

#include <algorithm>

#include "services/composition/ServiceBuilder.h"

using namespace AmbientCharacterBehavior;

bool BehavioralEntity::CanUpdate() const
{
    if (sequences.empty())
    {
        return !is_processing;
    }

    return !is_processing && sequences.top()->GetSequenceState() != SequenceState::WAITING_FOR_ACTION;
}

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
    is_processing = true;

    if (sequences.empty())
    {
        HandleEmptySequences();
        return;
    }

    logger.LogInfo("Executing sequence for entity: " + std::to_string(entity_id), "BehavioralEntity");

    switch (sequences.top()->GetSequenceState())
    {
        case SequenceState::UNINITIALIZED:
            HandleSequenceStartup();
            break;
        case SequenceState::PROCESSING_NODE:
            ExecuteCurrentNode();
            break;
        case SequenceState::IN_SUBSEQUENCE:
            HandleSubsequenceCompletion();
            break;
        case SequenceState::WAITING_FOR_ACTION:
            logger.LogInfo("Waiting for character '" + std::to_string(entity_id) + "' to complete action with id: " +
                std::to_string(current_action_id), "BehavioralEntity");
            break;
        case SequenceState::NODE_EXECUTED:
            HandleNodeExecutionCompletion();
            break;
        case SequenceState::FAILED:
            HandleSequenceFailure();
            break;
        case SequenceState::INTERRUPTED:
            HandleInterruptionRecovery();
            break;
    }

    is_processing = false;
}

void BehavioralEntity::HandleEmptySequences()
{
    if (main_sequence == nullptr)
    {
        logger.LogError("character with id: " + std::to_string(entity_id) + " does not have a valid main sequence.",
            "BehavioralEntity");

        // is processing is turned true to prevent the character from receiving future updates in this invalid state.
        is_processing = true;
        return;
    }

    sequences.emplace(main_sequence);
    sequences.top()->SetSequenceState(SequenceState::UNINITIALIZED);
    is_processing = false;
}


void BehavioralEntity::HandleSequenceStartup()
{
    logger.LogInfo("Handling sequence startup for entity: " + std::to_string(entity_id), "BehavioralEntity");
    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
}

void BehavioralEntity::ExecuteCurrentNode()
{
    //todo: implement full logic
    current_action_id = 1;
    current_action_token++;

    logger.LogInfo("Calling start character action for entity: " + std::to_string(entity_id) + " with action id: " +
        std::to_string(current_action_id) + " and token: " + std::to_string(current_action_token), "BehavioralEntity");

    start_character_action_provider.StartCharacterAction(entity_handle, current_action_id, current_action_token,
        nullptr);


    sequences.top()->SetSequenceState(SequenceState::WAITING_FOR_ACTION);
}

void BehavioralEntity::HandleSubsequenceCompletion()
{
}

void BehavioralEntity::HandleNodeExecutionCompletion()
{
    // EvaluateTransitions();
    // SelectBestTransition();
    // memory.UpdateTransitionMemory();
    // sequences.top()->SetSequenceState();
    // sequences.top()->GetCurrentNode().ResetCompletion();
    logger.LogInfo("Completed node execution for entity " + std::to_string(entity_id) + ". Will reset sequence to "
        "PROCESSING_NODE state", "BehavioralEntity");

    sequences.top()->SetSequenceState(SequenceState::PROCESSING_NODE);
}

void BehavioralEntity::HandleSequenceFailure()
{
}

void BehavioralEntity::HandleInterruptionRecovery()
{
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
        logger.LogInfo("entity with id: " + std::to_string(entity_id) + " has completed action with id: " +
            std::to_string(action_id) + " and token: " + std::to_string(action_token) ,"BehavioralEntity");
    }

    //todo: this should later be put inside of the if statement
    if (!sequences.empty())
    {
        sequences.top()->SetSequenceState(SequenceState::NODE_EXECUTED);
        logger.LogInfo("entity with id: " + std::to_string(entity_id) + " is now in NODE_EXECUTED step",
            "BehavioralEntity");
    }
    else
    {
        HandleEmptySequences();
    }

    is_processing = false;
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
