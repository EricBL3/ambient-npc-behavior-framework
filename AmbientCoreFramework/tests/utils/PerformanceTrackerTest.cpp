#include <gtest/gtest.h>
#include "utils/PerformanceTracker.h"
#include <chrono>
#include <thread>

using namespace AmbientCharacterBehavior;

class PerformanceTrackerTest : public testing::Test {
protected:
    void SetUp() override
    {
        PerformanceTracker::Reset();
    }

    void TearDown() override
    {
        PerformanceTracker::Reset();
    }
};


TEST_F(PerformanceTrackerTest, InitialState)
{
    EXPECT_FALSE(PerformanceTracker::IsTiming());
    EXPECT_EQ(PerformanceTracker::GetLastDurationMicroseconds(), 0.0);
}


TEST_F(PerformanceTrackerTest, StartStopState) {
    // Should not be timing initially
    EXPECT_FALSE(PerformanceTracker::IsTiming());

    PerformanceTracker::StartTiming();

    // Should be timing after start
    EXPECT_TRUE(PerformanceTracker::IsTiming());

    PerformanceTracker::StopTiming();

    // Should not be timing after end
    EXPECT_FALSE(PerformanceTracker::IsTiming());
}

TEST_F(PerformanceTrackerTest, ResetFunctionality) {
    PerformanceTracker::StartTiming();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    PerformanceTracker::StopTiming();

    // Should have a measured duration
    EXPECT_GT(PerformanceTracker::GetLastDurationMicroseconds(), 0.0);

    PerformanceTracker::Reset();

    // After reset, should be back to initial state
    EXPECT_FALSE(PerformanceTracker::IsTiming());
    EXPECT_EQ(PerformanceTracker::GetLastDurationMicroseconds(), 0.0);
}

// Edge case tests
TEST_F(PerformanceTrackerTest, EndWithoutStart) {
    // Calling EndTiming without StartTiming should not crash
    EXPECT_NO_THROW(PerformanceTracker::StopTiming());

    // Should return 0 duration
    EXPECT_EQ(PerformanceTracker::GetLastDurationMicroseconds(), 0.0);

    // Should not be in timing state
    EXPECT_FALSE(PerformanceTracker::IsTiming());
}


TEST_F(PerformanceTrackerTest, QuickTiming) {
    PerformanceTracker::StartTiming();

    // Do minimal work (just some arithmetic)
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i;
    }

    PerformanceTracker::StopTiming();

    double measured = PerformanceTracker::GetLastDurationMicroseconds();

    // Should measure something, even if very fast
    EXPECT_GE(measured, 0.0);
    // Should not be unreasonably high
    EXPECT_LT(measured, 1000.0);  // Less than 1ms for simple arithmetic
}
