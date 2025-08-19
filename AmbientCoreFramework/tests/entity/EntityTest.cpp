/**
 * @file EntityTest.cpp
 * @brief Unit tests for the entity class
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 *
*/
#include <gtest/gtest.h>

#include "entity/Entity.h"

using namespace AmbientCharacterBehavior;

class EntityTest : public testing::Test {
protected:
    void SetUp() override {
        entity = std::make_unique<Entity>(0, 0, "Eric");
    }

    std::unique_ptr<Entity> entity;
};

// =============================================================================
// CONSTRUCTION TESTS
// =============================================================================

TEST_F(EntityTest, ConstructorWorksProperly)
{
    EXPECT_EQ(entity->GetEntityId(), 0);
    EXPECT_EQ(entity->GetCurrentLocationId(), 0);
    EXPECT_EQ(entity->GetName(), "Eric");
}

TEST_F(EntityTest, ConstructorRejectsNegativeValues)
{
    EXPECT_THROW(Entity(-1, 0), std::invalid_argument);
    EXPECT_THROW(Entity(0, -1), std::invalid_argument);
}

// =============================================================================
// SET CURRENT LOCATION ID TESTS
// =============================================================================

TEST_F(EntityTest, SetCurrentLocationIdWorksProperly)
{
    EXPECT_EQ(entity->GetCurrentLocationId(), 0);

    entity->SetCurrentLocationId(1);

    EXPECT_EQ(entity->GetCurrentLocationId(), 1);
}

TEST_F(EntityTest, SetCurrentLocationIdRejectsNegativeValues)
{
   EXPECT_THROW(entity->SetCurrentLocationId(-1), std::invalid_argument);
}