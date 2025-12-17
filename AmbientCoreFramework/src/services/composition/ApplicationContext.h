#pragma once
#include <memory>

#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IEnvironmentalConditionProvider.h"
#include "services/interfaces/IFrameworkSchemaManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"
#include "ServiceBundles.h"
#include "services/interfaces/IEntityPositionManager.h"
#include "services/interfaces/IEntityPositionProvider.h"
#include "services/layers/4_content_registry/FrameworkRegistry.h"


namespace AmbientCharacterBehavior {
class ApplicationContext {
private:
    // Service ownership (order matters for destruction)
    std::unique_ptr<ILogger> logger;
    std::unique_ptr<ITimeManager> time_manager;
    std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider;
    std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider;
    std::unique_ptr<IEntityPositionProvider> entity_position_provider;
    std::unique_ptr<IJsonLoader> json_loader;
    std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager;
    std::unique_ptr<IEntityPositionManager> entity_position_manager;
    std::unique_ptr<IFrameworkSchemaManager> schema_manager;
    std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator;
    std::unique_ptr<FrameworkRegistry> registry;

    FoundationServices foundation_services;
    DataAccessServices data_access_services;
    SimulationServices simulation_state_services;
    BehavioralEvaluationServices behavioral_evaluation_services;
    ContentRegistryServices content_registry_services;

public:
    ApplicationContext(
        std::unique_ptr<ILogger> logger,
        std::unique_ptr<ITimeManager> time_manager,
        std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider,
        std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider,
        std::unique_ptr<IEntityPositionProvider> entity_position_provider,
        std::unique_ptr<IJsonLoader> json_loader,
        std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager,
        std::unique_ptr<IEntityPositionManager> entity_position_manager,
        std::unique_ptr<IFrameworkSchemaManager> schema_manager,
        std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator,
        std::unique_ptr<FrameworkRegistry> registry) :
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
        registry(std::move(registry)),
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
        content_registry_services(
            behavioral_evaluation_services,
            *this->registry,
            *this->registry,
            *this->registry
        ) {}

    FoundationServices& Foundation() { return foundation_services; }
    DataAccessServices& DataAccess() { return data_access_services; }
    SimulationServices& SimulationState() { return simulation_state_services; }
    BehavioralEvaluationServices& BehavioralEvaluation() { return behavioral_evaluation_services; }
    ContentRegistryServices& ContentRegistry() { return content_registry_services; }
};
}
