#include "ApplicationContext.h"
#include "ServiceFactories.h"

using namespace AmbientCharacterBehavior;

std::unique_ptr<ApplicationContext> ApplicationContext::Create(
    QueryEnvironmentalConditionCallback query_env_callback,
    StartCharacterActionCallback start_action_callback,
    QueryEntityPositionCallback query_position_callback
)
{
    using namespace ServiceFactories;

    // Layer 0 - Foundation
    auto logger = CreateLogger();
    auto time_manager = CreateTimeManager();
    auto env_provider = CreateEnvironmentalConditionProvider(query_env_callback);
    auto action_provider = CreateStartCharacterActionProvider(start_action_callback);
    auto pos_provider = CreateEntityPositionProvider(query_position_callback);

    // Layer 1 - Data Access
    auto json_loader = CreateJsonLoader(*logger);

    // Layer 2 - Simulation State
    auto env_manager = CreateEnvironmentalConditionManager(
        *logger, *time_manager, *json_loader, *env_provider);

    auto pos_manager = CreateEntityPositionManager(*logger, *time_manager, *pos_provider);
    auto schema_manager = CreateFrameworkSchemaManager(*logger, *json_loader);

    // Layer 3 - Behavioral Evaluation
    auto state_evaluator = CreateStateOperationEvaluator(
        *logger, *schema_manager, *env_manager, *pos_manager);

    // Shows Allocated memory is leaked warning
    auto context = std::unique_ptr<ApplicationContext>(new ApplicationContext(
        std::move(logger),
        std::move(time_manager),
        std::move(env_provider),
        std::move(action_provider),
        std::move(pos_provider),
        std::move(json_loader),
        std::move(env_manager),
        std::move(pos_manager),
        std::move(schema_manager),
        std::move(state_evaluator)
    ));


    // Layer 4 - Registry (needs ApplicationContext to exist to create with bundle reference)
    context->InitializeRegistry();

    return context;
}

ApplicationContext::ApplicationContext(
    std::unique_ptr<ILogger> logger,
    std::unique_ptr<ITimeManager> time_manager,
    std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider,
    std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider,
    std::unique_ptr<IEntityPositionProvider> entity_position_provider,
    std::unique_ptr<IJsonLoader> json_loader,
    std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager,
    std::unique_ptr<IEntityPositionManager> entity_position_manager,
    std::unique_ptr<IFrameworkSchemaManager> schema_manager,
    std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator
) :
    logger(std::move(logger)),
    time_manager(std::move(time_manager)),
    environmental_condition_provider(std::move(environmental_condition_provider)),
    start_character_action_provider(std::move(start_character_action_provider)),
    entity_position_provider(std::move(entity_position_provider)),
    json_loader(std::move(json_loader)),
    environmental_condition_manager(std::move(environmental_condition_manager)),
    entity_position_manager(std::move(entity_position_manager)),
    schema_manager(std::move(schema_manager)),
    state_operation_evaluator(std::move(state_operation_evaluator)),
    registry(nullptr), // Initialized in InitializeRegistry
    foundation_services(
        *this->logger,
        *this->time_manager,
        *this->environmental_condition_provider,
        *this->start_character_action_provider,
        *this->entity_position_provider
    ),
    data_access_services(
        foundation_services,
        *this->json_loader
    ),
    simulation_state_services(
        data_access_services,
        *this->environmental_condition_manager,
        *this->entity_position_manager,
        *this->schema_manager
    ),
    behavioral_evaluation_services(
        simulation_state_services,
        *this->state_operation_evaluator
    ),
    content_registry_services(nullptr) // Initialized in InitializeRegistry
    {}

void ApplicationContext::InitializeRegistry()
{
    registry = ServiceFactories::CreateFrameworkRegistry(behavioral_evaluation_services);

    content_registry_services = std::make_unique<ContentRegistryServices>(
        behavioral_evaluation_services,
        *registry,
        *registry,
        *registry
    );

    registry->SetSelfBundle(*content_registry_services);
}
