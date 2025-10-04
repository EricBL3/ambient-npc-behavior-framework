#pragma once
#include <memory>

#include "ApplicationContext.h"
#include "interfaces/IEnvironmentalConditionManager.h"
#include "interfaces/IEnvironmentalConditionProvider.h"
#include "interfaces/IFrameworkSchemaManager.h"
#include "interfaces/IJsonLoader.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStateOperationEvaluator.h"
#include "interfaces/ITimeManager.h"
#include "integration/BehaviorFramework.h"
#include "services/registry/FrameworkRegistry.h"

namespace AmbientCharacterBehavior {
class ServiceBuilder {
private:
    std::unique_ptr<ILogger> logger;
    std::unique_ptr<ITimeManager> time_manager;
    std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider;
    std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider;
    std::unique_ptr<IJsonLoader> json_loader;
    std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager;
    std::unique_ptr<IFrameworkSchemaManager> schema_manager;
    std::unique_ptr<FrameworkRegistry> registry;
    std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator;

public:
    ServiceBuilder& WithLogger(std::unique_ptr<ILogger> new_logger);
    ServiceBuilder& WithTimeManager(std::unique_ptr<ITimeManager> new_time_manager);
    ServiceBuilder& WithEnvironmentalConditionProvider(std::unique_ptr<IEnvironmentalConditionProvider> new_environmental_condition_provider);
    ServiceBuilder& WithStartCharacterActionProvider(std::unique_ptr<IStartCharacterActionProvider> new_start_character_action_provider);
    ServiceBuilder& WithJsonLoader();
    ServiceBuilder& WithEnvironmentalConditionManager();
    ServiceBuilder& WithSchemaManager();
    ServiceBuilder& WithFrameworkRegistry();
    ServiceBuilder& WithStateOperationEvaluator();

    std::unique_ptr<ApplicationContext> Build();

    static std::unique_ptr<ApplicationContext> CreateApplicationContext();
    static std::unique_ptr<BehaviorFramework> CreateBehaviorFramework();

private:
    void EnsureCoreServices() const;
    void EnsureConfigurationServices() const;
    void EnsureDomainServices() const;
    void EnsureRegistryServices() const;
    void EnsureAllServices() const;
};
}
