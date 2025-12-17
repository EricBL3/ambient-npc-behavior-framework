#pragma once
#include <memory>

#include "ServiceBundles.h"
#include "services/interfaces/IEntityPositionManager.h"
#include "services/interfaces/IEntityPositionProvider.h"
#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IEnvironmentalConditionProvider.h"
#include "services/interfaces/IFrameworkSchemaManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStartCharacterActionProvider.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"
#include "services/layers/0_foundation/FrameworkLogger.h"
#include "services/layers/0_foundation/TimeManager.h"
#include "services/layers/0_foundation/callback_providers/EntityPositionProvider.h"
#include "services/layers/0_foundation/callback_providers/EnvironmentalConditionProvider.h"
#include "services/layers/0_foundation/callback_providers/StartCharacterActionProvider.h"
#include "services/layers/1_data_access/JsonLoader.h"
#include "services/layers/2_simulation/EntityPositionManager.h"
#include "services/layers/2_simulation/EnvironmentalConditionManager.h"
#include "services/layers/2_simulation/FrameworkSchemaManager.h"
#include "services/layers/3_behavioral_evaluation/StateOperationEvaluator.h"
#include "services/layers/4_content_registry/FrameworkRegistry.h"

namespace AmbientCharacterBehavior {

class ApplicationContext;

namespace ServiceFactories {

    // Layer 0 - Foundation

    inline std::unique_ptr<ILogger> CreateLogger()
    {
        return std::make_unique<FrameworkLogger>();
    }

    inline std::unique_ptr<ITimeManager> CreateTimeManager()
    {
        return std::make_unique<TimeManager>();
    }

    inline std::unique_ptr<IEnvironmentalConditionProvider> CreateEnvironmentalConditionProvider(
        QueryEnvironmentalConditionCallback callback)
    {
        return std::make_unique<EnvironmentalConditionProvider>(callback);
    }

    inline std::unique_ptr<IStartCharacterActionProvider> CreateStartCharacterActionProvider(
        StartCharacterActionCallback callback)
    {
        return std::make_unique<StartCharacterActionProvider>(callback);
    }

    inline std::unique_ptr<IEntityPositionProvider> CreateEntityPositionProvider(
        QueryEntityPositionCallback callback)
    {
        return std::make_unique<EntityPositionProvider>(callback);
    }

    // Layer 1 - Data Access

    inline std::unique_ptr<IJsonLoader> CreateJsonLoader(ILogger& logger)
    {
        return std::make_unique<JsonLoader>(logger);
    }

    // Layer 2 - Simulation State

    inline std::unique_ptr<IEnvironmentalConditionManager> CreateEnvironmentalConditionManager(
        ILogger& logger,
        ITimeManager& time_manager,
        IJsonLoader& json_loader,
        IEnvironmentalConditionProvider& provider
    )
    {
        return std::make_unique<EnvironmentalConditionManager>(logger, time_manager, json_loader, provider);
    }

    inline std::unique_ptr<IEntityPositionManager> CreateEntityPositionManager(
        ILogger& logger,
        ITimeManager& time_manager,
        IEntityPositionProvider& provider
    )
    {
        return std::make_unique<EntityPositionManager>(logger, time_manager, provider);
    }

    inline std::unique_ptr<IFrameworkSchemaManager> CreateFrameworkSchemaManager(
        ILogger& logger,
        IJsonLoader& json_loader
    )
    {
        return std::make_unique<FrameworkSchemaManager>(logger, json_loader);
    }

    // Layer 3 - Behavioral Evaluation
    inline std::unique_ptr<IStateOperationEvaluator> CreateStateOperationEvaluator(
        ILogger& logger,
        IFrameworkSchemaManager& schema_manager,
        IEnvironmentalConditionManager& env_manager,
        IEntityPositionManager& pos_manager
    )
    {
        return std::make_unique<StateOperationEvaluator>(logger, schema_manager, env_manager, pos_manager);
    }

    // Layer 4 - Content Registry
    inline std::unique_ptr<FrameworkRegistry> CreateFrameworkRegistry(BehavioralEvaluationServices& behavioral_evaluation)
    {
        return std::make_unique<FrameworkRegistry>(behavioral_evaluation);
    }
}
}
