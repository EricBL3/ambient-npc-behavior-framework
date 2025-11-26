#pragma once
#include "services/interfaces/IContentProvider.h"
#include "services/interfaces/IEntityQuery.h"
#include "services/interfaces/IEntityRegistry.h"
#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IEnvironmentalConditionProvider.h"
#include "services/interfaces/IFrameworkSchemaManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStartCharacterActionProvider.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"

namespace AmbientCharacterBehavior {

struct CoreServices {
    ILogger& logger;
    ITimeManager& time_manager;
    IEnvironmentalConditionProvider& environmental_condition_provider;
    IStartCharacterActionProvider& start_character_action_provider;

    CoreServices(ILogger& logger, ITimeManager& time_manager,
        IEnvironmentalConditionProvider& environmental_condition_provider, IStartCharacterActionProvider& start_character_action_provider) :
        logger(logger), time_manager(time_manager), environmental_condition_provider(environmental_condition_provider),
        start_character_action_provider(start_character_action_provider){}
};

struct ConfigurationServices {
    CoreServices& core_services;
    IJsonLoader& json_loader;

    ConfigurationServices(CoreServices& core_services, IJsonLoader& json_loader) :
        core_services(core_services), json_loader(json_loader) {}
};

struct DomainServices {
    ConfigurationServices& configuration_services;
    IEnvironmentalConditionManager& environmental_condition_manager;
    IFrameworkSchemaManager& schema_manager;

    DomainServices(ConfigurationServices& configuration_services, IEnvironmentalConditionManager& environmental_condition_manager,
        IFrameworkSchemaManager& schema_manager) : configuration_services(configuration_services),
        environmental_condition_manager(environmental_condition_manager), schema_manager(schema_manager) {}
};

struct ApplicationServices {
    DomainServices& domain_services;
    IStateOperationEvaluator& state_operation_evaluator;

    ApplicationServices(DomainServices& domain_services, IStateOperationEvaluator& state_operation_evaluator) :
        domain_services(domain_services), state_operation_evaluator(state_operation_evaluator) {}
};

struct RegistryServices {
    ApplicationServices& application_services;
    IContentProvider& content_provider;
    IEntityRegistry& entity_registry;
    IEntityQuery& entity_query;

    RegistryServices(ApplicationServices& application_services, IContentProvider& content_provider, IEntityRegistry& entity_registry,
        IEntityQuery& entity_query) :
        application_services(application_services), content_provider(content_provider), entity_registry(entity_registry),
        entity_query(entity_query) {}
};

}
