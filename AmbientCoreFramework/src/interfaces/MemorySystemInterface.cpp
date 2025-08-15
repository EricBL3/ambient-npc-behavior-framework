/**
 * @file MemorySystemInterface.cpp
 * @brief Implementation of public interface for MemorySystem operations
 * @author Eric Buitrón López
 * @date 8/15/2025
 */

#include "../include/MemorySystemInterface.h"

#include "memory/MemorySystem.h"
#include "utils/PerformanceTracker.h"

extern "C"
{
    AmbientCoreFramework_API MemorySystemHandle CreateMemorySystem(int max_transitions, int max_actions, int max_interruptions)
    {
        PerformanceTracker::StartTiming();
        try
        {
            auto memorySystem = new MemorySystem(max_transitions, max_actions, max_interruptions);
            PerformanceTracker::StopTiming();
            return static_cast<MemorySystemHandle>(memorySystem);
        }catch (...)
        {
            PerformanceTracker::StopTiming();
            return nullptr;
        }
    }

    AmbientCoreFramework_API void DestroyMemorySystem(MemorySystemHandle handle)
    {
        if (handle != nullptr)
        {
            PerformanceTracker::StartTiming();
            delete static_cast<MemorySystem*>(handle);
            PerformanceTracker::StopTiming();
        }
    }

    AmbientCoreFramework_API int GetLeastRecentlyVisitedNode(MemorySystemHandle handle, int *node_ids, int count)
    {
        if (handle == nullptr || node_ids == nullptr || count <= 0)
        {
            return -1;
        }

        PerformanceTracker::StartTiming();

        try
        {
            auto memorySystem = static_cast<MemorySystem*>(handle);

            // Create C++ vector from a C array of integers
            std::vector<int> nodes(node_ids, node_ids + count);

            auto result = memorySystem->GetLeastRecentlyVisitedNode(nodes);

            PerformanceTracker::StopTiming();
            return result;

        }catch (...)
        {
            PerformanceTracker::StopTiming();
            return -1;
        }
    }

    AmbientCoreFramework_API int GetLeastRecentlyUsedEntityForAction(MemorySystemHandle handle, int action_id, int *entity_ids, int count)
    {
        if (handle == nullptr || entity_ids == nullptr || count <= 0)
        {
            return -1;
        }

        PerformanceTracker::StartTiming();

        try
        {
            auto memorySystem = static_cast<MemorySystem*>(handle);
            std::vector<int> entities(entity_ids, entity_ids + count);
            auto result = memorySystem->GetLeastRecentlyUsedEntityForAction(action_id, entities);

            PerformanceTracker::StopTiming();
            return result;
        } catch (...)
        {
            PerformanceTracker::StopTiming();
            return -1;
        }
    }

    AmbientCoreFramework_API int UpdateTransitionMemory(MemorySystemHandle handle, int target_node_id, int current_time)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = memorySystem->UpdateTransitionMemory(target_node_id, current_time);

        PerformanceTracker::StopTiming();
        return static_cast<int>(result);
    }

    AmbientCoreFramework_API int UpdateActionMemory(MemorySystemHandle handle, int action_id, int target_entity_id, int current_time)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = memorySystem->UpdateActionMemory(action_id, target_entity_id, current_time);

        PerformanceTracker::StopTiming();
        return static_cast<int>(result);
    }

    AmbientCoreFramework_API int UpdateInterruptionMemory(MemorySystemHandle handle, int action_id, int sequence_id, int node_id, int entity_id, int current_time)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = memorySystem->UpdateInterruptionMemory(action_id, sequence_id, node_id, entity_id, current_time);

        PerformanceTracker::StopTiming();
        return static_cast<int>(result);
    }

    AmbientCoreFramework_API int FindTransitionMemory(MemorySystemHandle handle, int target_node_id, int *out_timestamp)
    {
        if (handle == nullptr || out_timestamp == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto memory = memorySystem->FindTransitionMemory(target_node_id);

        // Memory wasn't found
        if (memory == nullptr)
        {
            PerformanceTracker::StopTiming();
            return 0;
        }

        *out_timestamp = memory->GetLastUsedTime();

        PerformanceTracker::StopTiming();

        return 1;
    }

    AmbientCoreFramework_API int FindActionMemory(MemorySystemHandle handle, int action_id, int target_entity_id, int *out_timestamp)
    {
        if (handle == nullptr || out_timestamp == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto memory = memorySystem->FindActionMemory(action_id, target_entity_id);

        // Memory wasn't found
        if (memory == nullptr)
        {
            PerformanceTracker::StopTiming();
            return 0;
        }

        *out_timestamp = memory->GetLastUsedTime();

        PerformanceTracker::StopTiming();

        return 1;
    }

    AmbientCoreFramework_API int FindInterruptionMemory(MemorySystemHandle handle, int action_id, int sequence_id, int node_id, int *out_entity_id, int *out_timestamp)
    {
        if (handle == nullptr || out_entity_id == nullptr || out_timestamp == nullptr )
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto memory = memorySystem->FindInterruptionMemory(action_id, sequence_id, node_id);

        // Memory wasn't found
        if (memory == nullptr)
        {
            PerformanceTracker::StopTiming();
            return 0;
        }

        *out_entity_id = memory->GetInterruptedTargetEntityId();
        *out_timestamp = memory->GetLastUsedTime();

        PerformanceTracker::StopTiming();

        return 1;
    }

    AmbientCoreFramework_API void ClearAllMemories(MemorySystemHandle handle)
    {
        if (handle != nullptr)
        {
            PerformanceTracker::StartTiming();

            auto memorySystem = static_cast<MemorySystem*>(handle);
            memorySystem->ClearAllMemories();

            PerformanceTracker::StopTiming();
        }

    }

    AmbientCoreFramework_API void ClearSequenceInterruptionMemories(MemorySystemHandle handle, int sequence_id)
    {
        if (handle != nullptr)
        {
            PerformanceTracker::StartTiming();

            auto memorySystem = static_cast<MemorySystem*>(handle);
            memorySystem->ClearSequenceInterruptionMemories(sequence_id);

            PerformanceTracker::StopTiming();
        }

    }

    AmbientCoreFramework_API int GetTransitionMemoryCount(MemorySystemHandle handle)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = static_cast<int>(memorySystem->GetTransitionMemoryCount());

        PerformanceTracker::StopTiming();

        return result;
    }

    AmbientCoreFramework_API int GetActionMemoryCount(MemorySystemHandle handle)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = static_cast<int>(memorySystem->GetActionMemoryCount());

        PerformanceTracker::StopTiming();

        return result;
    }

    AmbientCoreFramework_API int GetInterruptionMemoryCount(MemorySystemHandle handle)
    {
        if (handle == nullptr)
        {
            return 0;
        }

        PerformanceTracker::StartTiming();

        auto memorySystem = static_cast<MemorySystem*>(handle);
        auto result = static_cast<int>(memorySystem->GetInterruptionMemoryCount());

        PerformanceTracker::StopTiming();

        return result;
    }
}

