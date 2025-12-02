#include <gtest/gtest.h>

#include "entity/FrameworkEntity.h"

using namespace AmbientCharacterBehavior;

class FrameworkEntityTest : public testing::Test {
protected:

    int test_handle = 1;

    void SetUp() override {
        entity = std::make_unique<FrameworkEntity>(&test_handle, 0, "Test");
    }

    std::unique_ptr<FrameworkEntity> entity;
};

// =============================================================================
// CONSTRUCTION TESTS
// =============================================================================

TEST_F(FrameworkEntityTest, ConstructorWorksProperly)
{
    EXPECT_EQ(entity->GetEntityId(), 0);
    EXPECT_EQ(entity->GetName(), "Test");
    EXPECT_EQ(entity->GetEntityHandle(), &test_handle);
}

TEST_F(FrameworkEntityTest, ConstructorRejectsNegativeValues)
{
    EXPECT_THROW(FrameworkEntity(nullptr, 0), std::invalid_argument);
    EXPECT_THROW(FrameworkEntity(&test_handle, -1), std::invalid_argument);
}
