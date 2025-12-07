#pragma once
#include "entity/FrameworkEntity.h"

namespace AmbientCharacterBehavior {

struct StateOperationContext {
    FrameworkEntity* self_entity;
    FrameworkEntity* target_entity;

    explicit StateOperationContext(FrameworkEntity* self, FrameworkEntity* target = nullptr)
        : self_entity(self), target_entity(target) {}
};
}
