#include "ServiceBuilder.h"

#include "services/application/StateOperationEvaluator.h"
#include "services/configuration/JsonLoader.h"
#include "services/core/EntityPositionProvider.h"
#include "services/core/EnvironmentalConditionProvider.h"
#include "services/core/FrameworkLogger.h"
#include "services/core/StartCharacterActionProvider.h"
#include "services/core/TimeManager.h"
#include "services/domain/EntityPositionManager.h"
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

ServiceBuilder& ServiceBuilder::WithQueryEnvironmentalConditionCallback(QueryEnvironmentalConditionFn callback)
{
    if (!callback) {
        throw std::invalid_argument("Query callback cannot be null");
    }
    query_env_callback = callback;
    return *this;
}

ServiceBuilder& ServiceBuilder::WithStartCharacterActionCallback(StartCharacterActionFn callback)
{
    if (!callback) {
        throw std::invalid_argument("Start action callback cannot be null");
    }
    start_action_callback = callback;
    return *this;
}

ServiceBuilder & ServiceBuilder::WithQueryEntityPositionCallback(QueryEntityPositionFn callback)
{
    if (!callback) {
        throw std::invalid_argument("Query entity position callback cannot be null");
    }
    query_entity_pos_callback = callback;
    return *this;
}

ServiceBuilder & ServiceBuilder::WithProviders()
{
    EnsureCoreServices();

    if (!query_env_callback || !start_action_callback || !query_entity_pos_callback) {
        throw std::runtime_error(
            "Callbacks must be set before creating providers. "
            "Call WithQueryEnvironmentalConditionCallback(), "
            "WithQueryEntityPositionCallback() and "
            "WithStartCharacterActionCallback() first.");
    }

    environmental_condition_provider = std::make_unique<EnvironmentalConditionProvider>(query_env_callback);
    start_character_action_provider = std::make_unique<StartCharacterActionProvider>(start_action_callback);
    entity_pos_provider = std::make_unique<EntityPositionProvider>(query_entity_pos_callback);

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

ServiceBuilder & ServiceBuilder::WithEntityPositionManager()
{
    EnsureConfigurationServices();
    entity_position_manager = std::make_unique<EntityPositionManager>(*logger, *time_manager,
        *entity_pos_provider);

    return *this;
}

ServiceBuilder & ServiceBuilder::WithSchemaManager()
{
    EnsureConfigurationServices();
    schema_manager = std::make_unique<FrameworkSchemaManager>(*logger, *json_loader);
    return *this;
}

ServiceBuilder & ServiceBuilder::WithStateOperationEvaluator()
{
    EnsureDomainServices();
    state_operation_evaluator = std::make_unique<StateOperationEvaluator>(*logger, *schema_manager,
        *environmental_condition_manager, *entity_position_manager);

    return *this;
}

ServiceBuilder & ServiceBuilder::WithFrameworkRegistry()
{
    EnsureApplicationServices();
    registry = std::make_unique<FrameworkRegistry>(*logger, *time_manager, *start_character_action_provider, *json_loader,
        *schema_manager, *environmental_condition_manager, *state_operation_evaluator);

    return *this;
}



std::unique_ptr<ApplicationContext> ServiceBuilder::Build()
{
    EnsureAllServices();
    return std::make_unique<ApplicationContext>(
        std::move(logger),
        std::move(time_manager),
        std::move(environmental_condition_provider),
        std::move(start_character_action_provider),
        std::move(entity_pos_provider),
        std::move(json_loader),
        std::move(environmental_condition_manager),
        std::move(entity_position_manager),
        std::move(schema_manager),
        std::move(state_operation_evaluator),
        std::move(registry)
    );
}

std::unique_ptr<ApplicationContext> ServiceBuilder::CreateApplicationContext(QueryEnvironmentalConditionFn query_callback,
    StartCharacterActionFn start_action_callback, QueryEntityPositionFn query_position_callback)
{
    if (!query_callback || !start_action_callback || !query_position_callback) {
        throw std::invalid_argument(
            "CreateApplicationContext: Callbacks cannot be null");
    }

    return ServiceBuilder()
        .WithLogger(std::make_unique<FrameworkLogger>())
        .WithTimeManager(std::make_unique<TimeManager>())
        .WithQueryEnvironmentalConditionCallback(query_callback)
        .WithStartCharacterActionCallback(start_action_callback)
        .WithQueryEntityPositionCallback(query_position_callback)
        .WithProviders()
        .WithJsonLoader()
        .WithEnvironmentalConditionManager()
        .WithEntityPositionManager()
        .WithSchemaManager()
        .WithStateOperationEvaluator()
        .WithFrameworkRegistry()
        .Build();
}

std::unique_ptr<BehaviorFramework> ServiceBuilder::CreateBehaviorFramework(QueryEnvironmentalConditionFn query_callback,
    StartCharacterActionFn start_action_callback, QueryEntityPositionFn query_position_callback)
{
    return std::make_unique<BehaviorFramework>(CreateApplicationContext(query_callback, start_action_callback, query_position_callback));
}

void ServiceBuilder::EnsureCoreServices() const
{
    if (!logger || !time_manager)
    {
        throw std::runtime_error("Core services must be configured first");
    }
}

void ServiceBuilder::EnsureProvidersConfigured() const
{
    EnsureCoreServices();
    if (!environmental_condition_provider || !start_character_action_provider || !entity_pos_provider)
    {
        throw std::runtime_error("Providers must be configured first. Call WithProviders()");
    }
}

void ServiceBuilder::EnsureConfigurationServices() const
{
    EnsureProvidersConfigured();
    if (!json_loader)
    {
        throw std::runtime_error("JsonLoader must be configured first");
    }
}

void ServiceBuilder::EnsureDomainServices() const
{
    EnsureConfigurationServices();
    if (!environmental_condition_manager || !entity_position_manager || !schema_manager)
    {
        throw std::runtime_error("Domain services must be configured first");
    }
}

void ServiceBuilder::EnsureApplicationServices() const
{
    EnsureDomainServices();
    if (!state_operation_evaluator)
    {
        throw std::runtime_error("State operation evaluator must be configured first");
    }
}

void ServiceBuilder::EnsureAllServices() const
{
    EnsureApplicationServices();
    if (!registry)
    {
        throw std::runtime_error("Framework registry must be configured first");
    }
}
