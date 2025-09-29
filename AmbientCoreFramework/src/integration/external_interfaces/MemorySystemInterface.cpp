#include "../include/MemorySystemInterface.h"

#include "memory/MemorySystem.h"
#include "services/core/FrameworkLogger.h"
#include "utils/PerformanceTracker.h"

using namespace AmbientCharacterBehavior;

extern "C"
{
    AmbientCoreFramework_API MemorySystemHandle CreateMemorySystem(
        int32_t max_transitions,
        int32_t max_actions,
        int32_t max_interruptions
    )
    {
        try
        {
            auto logger = FrameworkLogger();
            auto memorySystem = new MemorySystem(max_transitions, max_actions, max_interruptions, logger);
            return static_cast<MemorySystemHandle>(memorySystem);
        }catch (...)
        {
            return nullptr;
        }
    }

    AmbientCoreFramework_API void DestroyMemorySystem(MemorySystemHandle handle)
    {
        if (handle != nullptr)
        {
            delete static_cast<MemorySystem*>(handle);
        }
    }

    AmbientCoreFramework_API int32_t GetLeastRecentlyVisitedNode(
        MemorySystemHandle handle,
        int32_t *node_ids,
        int32_t count
    )
    {
        if (handle == nullptr || node_ids == nullptr || count <= 0)
        {
            return -1;
        }


        try
        {
            auto memorySystem = static_cast<MemorySystem*>(handle);

            // Create C++ vector from a C array of integers
            std::vector<int32_t> nodes(node_ids, node_ids + count);

            int32_t result = memorySystem->GetLeastRecentlyVisitedNodeId(nodes);

            return result;

        }catch (...)
        {
            return -1;
        }
    }

    AmbientCoreFramework_API int32_t GetLeastRecentlyUsedEntityForAction(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t *entity_ids,
        int32_t count
    )
    {
        if (handle == nullptr || entity_ids == nullptr || count <= 0)
        {
            return -1;
        }


        try
        {
            auto memorySystem = static_cast<MemorySystem*>(handle);
            std::vector<int32_t> entities(entity_ids, entity_ids + count);
            int32_t result = memorySystem->GetLeastRecentlyUsedEntityIdForAction(action_id, entities);

            return result;
        } catch (...)
        {
            return -1;
        }
    }

    AmbientCoreFramework_API bool UpdateTransitionMemory(
        MemorySystemHandle handle,
        int32_t target_node_id,
        int64_t current_time
    )
    {
        if (handle == nullptr)
        {
            return 0;
        }


        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = memorySystem->UpdateTransitionMemory(target_node_id, current_time);

        return result;
    }

    AmbientCoreFramework_API bool UpdateActionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t target_entity_id,
        int64_t current_time
    )
    {
        if (handle == nullptr)
        {
            return 0;
        }


        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = memorySystem->UpdateActionMemory(action_id, target_entity_id, current_time);

        return result;
    }

    AmbientCoreFramework_API bool UpdateInterruptionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t sequence_id,
        int32_t node_id,
        int32_t entity_id,
        int64_t current_time
    )
    {
        if (handle == nullptr)
        {
            return 0;
        }


        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = memorySystem->UpdateInterruptionMemory(action_id, sequence_id, node_id, entity_id, current_time);

        return result;
    }

    AmbientCoreFramework_API bool FindTransitionMemory(
        MemorySystemHandle handle,
        int32_t target_node_id,
        int64_t *out_timestamp
    )
    {
        if (handle == nullptr || out_timestamp == nullptr)
        {
            return false;
        }


        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto memory = memorySystem->FindTransitionMemory(target_node_id);

        // Memory wasn't found
        if (memory == nullptr)
        {
            return false;
        }

        *out_timestamp = memory->GetLastUsedTime();


        return true;
    }

    AmbientCoreFramework_API bool FindActionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t target_entity_id,
        int64_t *out_timestamp
    )
    {
        if (handle == nullptr || out_timestamp == nullptr)
        {
            return false;
        }


        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto memory = memorySystem->FindActionMemory(action_id, target_entity_id);

        // Memory wasn't found
        if (memory == nullptr)
        {
            return false;
        }

        *out_timestamp = memory->GetLastUsedTime();

        return true;
    }

    AmbientCoreFramework_API bool FindInterruptionMemory(
        MemorySystemHandle handle,
        int32_t action_id,
        int32_t sequence_id,
        int32_t node_id,
        int32_t *out_entity_id,
        int64_t *out_timestamp
    )
    {
        if (handle == nullptr || out_entity_id == nullptr || out_timestamp == nullptr )
        {
            return false;
        }

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto memory = memorySystem->FindInterruptionMemory(action_id, sequence_id, node_id);

        // Memory wasn't found
        if (memory == nullptr)
        {
            return false;
        }

        *out_entity_id = memory->GetInterruptedTargetEntityId();
        *out_timestamp = memory->GetLastUsedTime();

        return true;
    }

    AmbientCoreFramework_API void ClearAllMemories(MemorySystemHandle handle)
    {
        if (handle != nullptr)
        {
            auto memorySystem = static_cast<MemorySystem*>(handle);
            memorySystem->ClearAllMemories();

        }

    }

    AmbientCoreFramework_API void ClearSequenceInterruptionMemories(MemorySystemHandle handle, int32_t sequence_id)
    {
        if (handle != nullptr)
        {
            auto memorySystem = static_cast<MemorySystem*>(handle);
            memorySystem->ClearSequenceInterruptionMemories(sequence_id);

        }

    }

    AmbientCoreFramework_API int32_t GetTransitionMemoryCount(MemorySystemHandle handle)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = static_cast<int32_t>(memorySystem->GetTransitionMemoryCount());

        return result;
    }

    AmbientCoreFramework_API int32_t GetActionMemoryCount(MemorySystemHandle handle)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = static_cast<int32_t>(memorySystem->GetActionMemoryCount());

        return result;
    }

    AmbientCoreFramework_API int32_t GetInterruptionMemoryCount(MemorySystemHandle handle)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = static_cast<int32_t>(memorySystem->GetInterruptionMemoryCount());

        return result;
    }
}

