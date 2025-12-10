#include <gtest/gtest.h>

#include "../../mocks/MockEnvironmentalConditionProvider.h"
#include "../../mocks/MockStartCharacterActionProvider.h"
#include "../../mocks/MockLogger.h"
#include "../../mocks/MockTimeManager.h"
#include "integration/BehaviorFramework.h"
#include "services/composition/ServiceBuilder.h"

using namespace AmbientCharacterBehavior;

static int32_t TestQueryEnvironmentalCondition(int32_t condition_key)
{
    return 42;
}

static void TestStartCharacterAction(void* entity_handle, int32_t action_id, int64_t action_token, int64_t action_duration_ms,
    void* target_entity_handle)
{

}

static bool TestQueryEntityPosition(void* entity_handle, int32_t* out_xyz)
{
    return true;
}

class ServiceBuilderTest : public testing::Test {
protected:
    std::unique_ptr<BehaviorFramework> framework;
};

TEST_F(ServiceBuilderTest, CreateBehaviorFramework_ProducesValidFramework) {
    EXPECT_NO_THROW({
        framework = ServiceBuilder::CreateBehaviorFramework(
            TestQueryEnvironmentalCondition,
            TestStartCharacterAction,
            TestQueryEntityPosition
        );
    });

    ASSERT_NE(framework, nullptr);
}


TEST_F(ServiceBuilderTest, CreateBehaviorFramework_WithNullQueryCallback_Throws) {
    EXPECT_THROW({
        framework = ServiceBuilder::CreateBehaviorFramework(
            nullptr,
            TestStartCharacterAction,
            TestQueryEntityPosition
        );
    }, std::invalid_argument);
}

TEST_F(ServiceBuilderTest, CreateBehaviorFramework_WithNullActionCallback_Throws) {
    EXPECT_THROW({
        framework = ServiceBuilder::CreateBehaviorFramework(
            TestQueryEnvironmentalCondition,
            nullptr,
            TestQueryEntityPosition
        );
    }, std::invalid_argument);
}

TEST_F(ServiceBuilderTest, CreateBehaviorFramework_WithNullEntityPositionCallback_Throws) {
    EXPECT_THROW({
        framework = ServiceBuilder::CreateBehaviorFramework(
            TestQueryEnvironmentalCondition,
            TestStartCharacterAction,
            nullptr
        );
    }, std::invalid_argument);
}