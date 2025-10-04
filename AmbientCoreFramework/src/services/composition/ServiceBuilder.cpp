#include "ServiceBuilder.h"

#include "services/application/StateOperationEvaluator.h"
#include "services/configuration/JsonLoader.h"
#include "services/core/EnvironmentalConditionProvider.h"
#include "services/core/FrameworkLogger.h"
#include "services/core/StartCharacterActionProvider.h"
#include "services/core/TimeManager.h"
#include "services/domain/EnvironmentalConditionManager.h"
#include "services/domain/FrameworkSchemaManager.h"
#include "services/registry/FrameworkRegistry.h"

using namespace AmbientCharacterBehavior;

ServiceBuilder & ServiceBuilder::WithLogger(std::unique_ptr<ILogger> new_logger)
{
    logger = std::move(new_logger);
    return *this;
}

ServiceBuilder & ServiceBuilder::WithTimeManager(std::unique_ptr<ITimeManager> new_time_manager)
{
    time_manager = std::move(new_time_manager);
    return *this;
}

ServiceBuilder & ServiceBuilder::WithEnvironmentalConditionProvider(
    std::unique_ptr<IEnvironmentalConditionProvider> new_environmental_condition_provider)
{
    environmental_condition_provider = std::move(new_environmental_condition_provider);
    return *this;
}

ServiceBuilder & ServiceBuilder::WithStartCharacterActionProvider(
    std::unique_ptr<IStartCharacterActionProvider> new_start_character_action_provider)
{
    start_character_action_provider = std::move(new_start_character_action_provider);
    return *this;
}


ServiceBuilder & ServiceBuilder::WithJsonLoader()
{
    EnsureCoreServices();
    json_loader = std::make_unique<JsonLoader>(*logger);
    return *this;
}

ServiceBuilder & ServiceBuilder::WithEnvironmentalConditionManager()
{
    EnsureConfigurationServices();
    environmental_condition_manager = std::make_unique<EnvironmentalConditionManager>(*logger, *time_manager,
        *json_loader,*environmental_condition_provider);

    return *this;
}

ServiceBuilder & ServiceBuilder::WithSchemaManager()
{
    EnsureConfigurationServices();
    schema_manager = std::make_unique<FrameworkSchemaManager>(*logger, *json_loader);
    return *this;
}


ServiceBuilder & ServiceBuilder::WithFrameworkRegistry()
{
    EnsureDomainServices();
    registry = std::make_unique<FrameworkRegistry>(*logger, *start_character_action_provider, *json_loader,
        *schema_manager, *environmental_condition_manager);

    return *this;
}

ServiceBuilder & ServiceBuilder::WithStateOperationEvaluator()
{
    EnsureRegistryServices();
    state_operation_evaluator = std::make_unique<StateOperationEvaluator>(*logger, *schema_manager,
        *environmental_condition_manager);

    return *this;
}

std::unique_ptr<ApplicationContext> ServiceBuilder::Build()
{
    EnsureAllServices();
    return std::make_unique<ApplicationContext>(std::move(logger), std::move(time_manager),
        std::move(environmental_condition_provider), std::move(start_character_action_provider),
        std::move(json_loader), std::move(environmental_condition_manager), std::move(schema_manager), std::move(registry),
        std::move(state_operation_evaluator));
}

std::unique_ptr<ApplicationContext> ServiceBuilder::CreateApplicationContext()
{
    return ServiceBuilder()
        .WithLogger(std::make_unique<FrameworkLogger>())
        .WithTimeManager(std::make_unique<TimeManager>())
        .WithEnvironmentalConditionProvider(std::make_unique<EnvironmentalConditionProvider>())
        .WithStartCharacterActionProvider(std::make_unique<StartCharacterActionProvider>())
        .WithJsonLoader()
        .WithEnvironmentalConditionManager()
        .WithSchemaManager()
        .WithFrameworkRegistry()
        .WithStateOperationEvaluator()
        .Build();
}

std::unique_ptr<BehaviorFramework> ServiceBuilder::CreateBehaviorFramework()
{
    return std::make_unique<BehaviorFramework>(CreateApplicationContext());
}

void ServiceBuilder::EnsureCoreServices() const
{
    if (!logger || !time_manager || !environmental_condition_provider || !start_character_action_provider)
    {
        throw std::runtime_error("Core services must be configured first");
    }
}

void ServiceBuilder::EnsureConfigurationServices() const
{
    EnsureCoreServices();
    if (!json_loader)
    {
        throw std::runtime_error("JsonLoader must be configured first");
    }
}

void ServiceBuilder::EnsureDomainServices() const
{
    EnsureConfigurationServices();
    if (!environmental_condition_manager || !schema_manager)
    {
        throw std::runtime_error("Domain services must be configured first");
    }
}

void ServiceBuilder::EnsureRegistryServices() const
{
    EnsureDomainServices();
    if (!registry)
    {
        throw std::runtime_error("Framework registry must be configured first");
    }
}

void ServiceBuilder::EnsureAllServices() const
{
    EnsureRegistryServices();
    if (!state_operation_evaluator)
    {
        throw std::runtime_error("State operation evaluator must be configured first");
    }
}
