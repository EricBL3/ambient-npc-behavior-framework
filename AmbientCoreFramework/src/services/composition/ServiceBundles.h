#pragma once
#include "services/interfaces/IActionTimeoutManager.h"
#include "services/interfaces/IContentProvider.h"
#include "services/interfaces/IEntityPositionManager.h"
#include "services/interfaces/IEntityPositionProvider.h"
#include "services/interfaces/IEntityQuery.h"
#include "services/interfaces/IEntityRegistry.h"
#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IEnvironmentalConditionProvider.h"
#include "services/interfaces/IFrameworkSchemaManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/ISelectionAlgorithmManager.h"
#include "services/interfaces/IStartCharacterActionProvider.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"

namespace AmbientCharacterBehavior {

struct FoundationServices {
    ILogger& logger;
    ITimeManager& time_manager;
    ISelectionAlgorithmManager& selection_algorithm_manager;
    IEnvironmentalConditionProvider& environmental_condition_provider;
    IStartCharacterActionProvider& start_character_action_provider;
    IEntityPositionProvider& entity_position_provider;

    FoundationServices(ILogger& logger, ITimeManager& time_manager, ISelectionAlgorithmManager& selection_algorithm_manager,
        IEnvironmentalConditionProvider& environmental_condition_provider,
        IStartCharacterActionProvider& start_character_action_provider, IEntityPositionProvider& entity_position_provider) :
        logger(logger), time_manager(time_manager), selection_algorithm_manager(selection_algorithm_manager),
        environmental_condition_provider(environmental_condition_provider),
        start_character_action_provider(start_character_action_provider),
        entity_position_provider(entity_position_provider){}
};

struct DataAccessServices {
    FoundationServices& foundation;
    IJsonLoader& json_loader;

    DataAccessServices(FoundationServices& foundation, IJsonLoader& json_loader) :
        foundation(foundation), json_loader(json_loader) {}
};

struct SimulationServices {
    DataAccessServices& data_access;
    IEnvironmentalConditionManager& environmental_condition_manager;
    IEntityPositionManager& entity_position_manager;
    IFrameworkSchemaManager& schema_manager;
    IActionTimeoutManager& action_timeout_manager;

    SimulationServices(DataAccessServices& data_access, IEnvironmentalConditionManager& environmental_condition_manager,
        IEntityPositionManager& entity_position_manager, IFrameworkSchemaManager& schema_manager,
        IActionTimeoutManager& action_timeout_manager) :
        data_access(data_access),
        environmental_condition_manager(environmental_condition_manager),
        entity_position_manager(entity_position_manager),
        schema_manager(schema_manager),
        action_timeout_manager(action_timeout_manager) {}
};

struct BehavioralEvaluationServices {
    SimulationServices& simulation_state;
    IStateOperationEvaluator& state_operation_evaluator;

    BehavioralEvaluationServices(SimulationServices& simulation_state, IStateOperationEvaluator& state_operation_evaluator) :
        simulation_state(simulation_state), state_operation_evaluator(state_operation_evaluator) {}
};

struct ContentRegistryServices {
    BehavioralEvaluationServices& behavioral_evaluation;
    IContentProvider& content_provider;
    IEntityRegistry& entity_registry;
    IEntityQuery& entity_query;

    ContentRegistryServices(BehavioralEvaluationServices& behavioral_evaluation, IContentProvider& content_provider,
        IEntityRegistry& entity_registry, IEntityQuery& entity_query) :
        behavioral_evaluation(behavioral_evaluation), content_provider(content_provider), entity_registry(entity_registry),
        entity_query(entity_query) {}
};

}
