#include <gtest/gtest.h>
#include "../../mocks/MockTimeManager.h"
#include "services/layers/0_foundation/TimeManager.h"

using namespace AmbientCharacterBehavior;

class TimeManagerTest : public testing::Test {
protected:

    std::unique_ptr<TimeManager> time_manager;

    void SetUp() override
    {
        time_manager = std::make_unique<TimeManager>();
    }
};

TEST_F(TimeManagerTest, GetCurrentTime_Works)
{
    EXPECT_EQ(0, time_manager->GetCurrentTime());
}

TEST_F(TimeManagerTest, SetCurrentTime_Works)
{
    time_manager->SetCurrentTime(100);

    EXPECT_EQ(100, time_manager->GetCurrentTime());
}

TEST_F(TimeManagerTest, SetCurrentTime_IgnoresNegativeValue)
{
    time_manager->SetCurrentTime(-100);
    EXPECT_EQ(0, time_manager->GetCurrentTime());
}