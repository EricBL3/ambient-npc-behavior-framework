#pragma once
#include "interfaces/IEnvironmentalConditionManager.h"
#include "interfaces/IEnvironmentalConditionProvider.h"
#include "interfaces/IFrameworkRegistry.h"
#include "interfaces/IFrameworkSchemaManager.h"
#include "interfaces/IJsonLoader.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStateOperationEvaluator.h"
#include "interfaces/ITimeManager.h"

namespace AmbientCharacterBehavior {

struct CoreServices {
    ILogger& logger;
    ITimeManager& time_manager;
    IEnvironmentalConditionProvider& environmental_condition_provider;

    CoreServices(ILogger& logger, ITimeManager& time_manager, IEnvironmentalConditionProvider& environmental_condition_provider) :
        logger(logger), time_manager(time_manager), environmental_condition_provider(environmental_condition_provider) {}
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

struct RegistryServices {
    DomainServices& domain_services;
    IFrameworkRegistry& registry;

    RegistryServices(DomainServices& domain_services, IFrameworkRegistry& registry) :
        domain_services(domain_services), registry(registry) {}
};

struct ApplicationServices {
    RegistryServices& registry;
    IStateOperationEvaluator& state_operation_evaluator;

    ApplicationServices(RegistryServices& registry, IStateOperationEvaluator& state_operation_evaluator) :
        registry(registry), state_operation_evaluator(state_operation_evaluator) {}
};

}
