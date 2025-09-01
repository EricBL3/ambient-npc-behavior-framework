/**
 * @file FrameworkEntityTest.cpp
 * @brief Unit tests for the entity class
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 *
*/
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

// =============================================================================
// ADD SUPPORTED ACTION TESTS
// =============================================================================

TEST_F(FrameworkEntityTest, AddSupportedActionWorksCorrectly)
{
    entity->AddSupportedAction(0);

    EXPECT_EQ(1, entity->GetSupportedActionsIds().size());
}

TEST_F(FrameworkEntityTest, AddSupportedActionDoesNothingOnRepeat)
{
    entity->AddSupportedAction(0);
    entity->AddSupportedAction(0);

    EXPECT_EQ(1, entity->GetSupportedActionsIds().size());
}

TEST_F(FrameworkEntityTest, AddSupportedActionRejectsInvalidActionId)
{
    EXPECT_THROW(entity->AddSupportedAction(-1), std::invalid_argument);
}


// =============================================================================
// REMOVE SUPPORTED ACTION TESTS
// =============================================================================

TEST_F(FrameworkEntityTest, RemoveSupportedActionWorksCorrectly)
{
    entity->AddSupportedAction(0);
    EXPECT_EQ(1, entity->GetSupportedActionsIds().size());

    entity->RemoveSupportedAction(0);
    EXPECT_EQ(0, entity->GetSupportedActionsIds().size());

}

TEST_F(FrameworkEntityTest, RemoveSupportedActionDoesNothingIfActionNotSupported)
{
    entity->AddSupportedAction(0);
    EXPECT_EQ(1, entity->GetSupportedActionsIds().size());

    entity->RemoveSupportedAction(5);
    EXPECT_EQ(1, entity->GetSupportedActionsIds().size());

}

// =============================================================================
// SUPPORTS ACTION TESTS
// =============================================================================

TEST_F(FrameworkEntityTest, SupportsActionReturnsTrueIfValid)
{
    entity->AddSupportedAction(0);
    EXPECT_EQ(true, entity->SupportsAction(0));
}

TEST_F(FrameworkEntityTest, SupportsActionReturnsFalseIfNotValid)
{
    EXPECT_EQ(false, entity->SupportsAction(5));
}
