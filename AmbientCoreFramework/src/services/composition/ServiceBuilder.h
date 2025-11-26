#pragma once
#include <memory>

#include "ApplicationContext.h"
#include "../../../include/BehaviorFrameworkInterface.h"
#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IEnvironmentalConditionProvider.h"
#include "services/interfaces/IFrameworkSchemaManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"
#include "integration/BehaviorFramework.h"
#include "services/registry/FrameworkRegistry.h"

namespace AmbientCharacterBehavior {
class ServiceBuilder {
private:
    QueryEnvironmentalConditionFn query_env_callback = nullptr;
    StartCharacterActionFn start_action_callback = nullptr;

    std::unique_ptr<ILogger> logger;
    std::unique_ptr<ITimeManager> time_manager;
    std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider;
    std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider;
    std::unique_ptr<IJsonLoader> json_loader;
    std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager;
    std::unique_ptr<IFrameworkSchemaManager> schema_manager;
    std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator;
    std::unique_ptr<FrameworkRegistry> registry;

public:
    ServiceBuilder& WithLogger(std::unique_ptr<ILogger> new_logger);
    ServiceBuilder& WithTimeManager(std::unique_ptr<ITimeManager> new_time_manager);
    ServiceBuilder& WithQueryEnvironmentalConditionCallback(QueryEnvironmentalConditionFn callback);
    ServiceBuilder& WithStartCharacterActionCallback(StartCharacterActionFn callback);
    ServiceBuilder& WithProviders();

    ServiceBuilder& WithJsonLoader();
    ServiceBuilder& WithEnvironmentalConditionManager();
    ServiceBuilder& WithSchemaManager();
    ServiceBuilder& WithStateOperationEvaluator();
    ServiceBuilder& WithFrameworkRegistry();

    std::unique_ptr<ApplicationContext> Build();

    static std::unique_ptr<ApplicationContext> CreateApplicationContext(QueryEnvironmentalConditionFn query_callback,
    StartCharacterActionFn start_action_callback);

    static std::unique_ptr<BehaviorFramework> CreateBehaviorFramework(QueryEnvironmentalConditionFn query_callback,
    StartCharacterActionFn start_action_callback);

private:
    void EnsureCoreServices() const;
    void EnsureProvidersConfigured() const;
    void EnsureConfigurationServices() const;
    void EnsureDomainServices() const;
    void EnsureApplicationServices() const;
    void EnsureAllServices() const;
};
}
