#include "ServiceBuilder.h"

#include "services/layers/3_behavioral_evaluation/StateOperationEvaluator.h"
#include "services/layers/1_data_access/JsonLoader.h"
#include "../layers/0_foundation/callback_providers/EntityPositionProvider.h"
#include "services/layers/0_foundation/callback_providers/EnvironmentalConditionProvider.h"
#include "services/layers/0_foundation/FrameworkLogger.h"
#include "services/layers/0_foundation/callback_providers/StartCharacterActionProvider.h"
#include "services/layers/0_foundation/TimeManager.h"
#include "services/layers/2_simulation/EntityPositionManager.h"
#include "services/layers/2_simulation/EnvironmentalConditionManager.h"
#include "services/layers/2_simulation/FrameworkSchemaManager.h"
#include "services/layers/4_content_registry/FrameworkRegistry.h"

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
    EnsureFoundationServices();

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
    EnsureFoundationServices();
    json_loader = std::make_unique<JsonLoader>(*logger);
    return *this;
}

ServiceBuilder & ServiceBuilder::WithEnvironmentalConditionManager()
{
    EnsureDataAccessServices();
    environmental_condition_manager = std::make_unique<EnvironmentalConditionManager>(*logger, *time_manager,
        *json_loader,*environmental_condition_provider);

    return *this;
}

ServiceBuilder & ServiceBuilder::WithEntityPositionManager()
{
    EnsureDataAccessServices();
    entity_position_manager = std::make_unique<EntityPositionManager>(*logger, *time_manager,
        *entity_pos_provider);

    return *this;
}

ServiceBuilder & ServiceBuilder::WithSchemaManager()
{
    EnsureDataAccessServices();
    schema_manager = std::make_unique<FrameworkSchemaManager>(*logger, *json_loader);
    return *this;
}

ServiceBuilder & ServiceBuilder::WithStateOperationEvaluator()
{
    EnsureSimulationStateServices();
    state_operation_evaluator = std::make_unique<StateOperationEvaluator>(*logger, *schema_manager,
        *environmental_condition_manager, *entity_position_manager);

    return *this;
}

ServiceBuilder & ServiceBuilder::WithFrameworkRegistry()
{
    EnsureBehavioralEvaluationServices();
    registry = std::make_unique<FrameworkRegistry>(*logger, *time_manager, *start_character_action_provider, *json_loader,
        *schema_manager, *environmental_condition_manager, *state_operation_evaluator, *entity_position_manager);

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

void ServiceBuilder::EnsureFoundationServices() const
{
    if (!logger || !time_manager)
    {
        throw std::runtime_error("Core services must be configured first");
    }
}

void ServiceBuilder::EnsureProvidersConfigured() const
{
    EnsureFoundationServices();
    if (!environmental_condition_provider || !start_character_action_provider || !entity_pos_provider)
    {
        throw std::runtime_error("Providers must be configured first. Call WithProviders()");
    }
}

void ServiceBuilder::EnsureDataAccessServices() const
{
    EnsureProvidersConfigured();
    if (!json_loader)
    {
        throw std::runtime_error("JsonLoader must be configured first");
    }
}

void ServiceBuilder::EnsureSimulationStateServices() const
{
    EnsureDataAccessServices();
    if (!environmental_condition_manager || !entity_position_manager || !schema_manager)
    {
        throw std::runtime_error("Domain services must be configured first");
    }
}

void ServiceBuilder::EnsureBehavioralEvaluationServices() const
{
    EnsureSimulationStateServices();
    if (!state_operation_evaluator)
    {
        throw std::runtime_error("State operation evaluator must be configured first");
    }
}

void ServiceBuilder::EnsureAllServices() const
{
    EnsureBehavioralEvaluationServices();
    if (!registry)
    {
        throw std::runtime_error("Framework registry must be configured first");
    }
}
