#include "PerformanceTracker.h"

using namespace AmbientCharacterBehavior;

PerformanceTracker::TimingSlot PerformanceTracker::update_timing;
PerformanceTracker::TimingSlot PerformanceTracker::complete_action_timing;
PerformanceTracker::TimingSlot PerformanceTracker::process_interruption_timing;
PerformanceTracker::TimingSlot PerformanceTracker::register_entity_timing;
PerformanceTracker::TimingSlot PerformanceTracker::unregister_entity_timing;

void PerformanceTracker::StartTiming(TimingSlot& slot) {
    slot.start_time = std::chrono::high_resolution_clock::now();
    slot.is_timing = true;
}

void PerformanceTracker::StopTiming(TimingSlot& slot) {
    if (!slot.is_timing) {
        slot.last_duration_microseconds = 0.0;
        return;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - slot.start_time).count();

    slot.last_duration_microseconds = static_cast<double>(duration);
    slot.is_timing = false;
}

double PerformanceTracker::GetDuration(const TimingSlot& slot) {
    return slot.last_duration_microseconds;
}

void PerformanceTracker::StartUpdateTiming() {
    StartTiming(update_timing);
}

void PerformanceTracker::StopUpdateTiming() {
    StopTiming(update_timing);
}

double PerformanceTracker::GetLastUpdateDurationMicroseconds() {
    return GetDuration(update_timing);
}

void PerformanceTracker::StartCompleteActionTiming() {
    StartTiming(complete_action_timing);
}

void PerformanceTracker::StopCompleteActionTiming() {
    StopTiming(complete_action_timing);
}

double PerformanceTracker::GetLastCompleteActionDurationMicroseconds() {
    return GetDuration(complete_action_timing);
}

// Interruption timing
void PerformanceTracker::StartProcessInterruptionTiming() {
    StartTiming(process_interruption_timing);
}

void PerformanceTracker::StopProcessInterruptionTiming() {
    StopTiming(process_interruption_timing);
}

double PerformanceTracker::GetLastProcessInterruptionDurationMicroseconds() {
    return GetDuration(process_interruption_timing);
}

// Entity registration timing
void PerformanceTracker::StartRegisterEntityTiming() {
    StartTiming(register_entity_timing);
}

void PerformanceTracker::StopRegisterEntityTiming() {
    StopTiming(register_entity_timing);
}

double PerformanceTracker::GetLastRegisterEntityDurationMicroseconds() {
    return GetDuration(register_entity_timing);
}

// Entity unregistration timing
void PerformanceTracker::StartUnregisterEntityTiming() {
    StartTiming(unregister_entity_timing);
}

void PerformanceTracker::StopUnregisterEntityTiming() {
    StopTiming(unregister_entity_timing);
}

double PerformanceTracker::GetLastUnregisterEntityDurationMicroseconds() {
    return GetDuration(unregister_entity_timing);
}

void PerformanceTracker::ResetAll() {
    update_timing = TimingSlot{};
    complete_action_timing = TimingSlot{};
    process_interruption_timing = TimingSlot{};
    register_entity_timing = TimingSlot{};
    unregister_entity_timing = TimingSlot{};
}
