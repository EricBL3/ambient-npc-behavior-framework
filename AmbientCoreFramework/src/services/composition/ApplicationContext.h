#pragma once
#include <memory>

#include "interfaces/IEnvironmentalConditionManager.h"
#include "interfaces/IEnvironmentalConditionProvider.h"
#include "interfaces/IFrameworkSchemaManager.h"
#include "interfaces/IJsonLoader.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStateOperationEvaluator.h"
#include "interfaces/ITimeManager.h"
#include "ServiceBundles.h"
#include "services/registry/FrameworkRegistry.h"


namespace AmbientCharacterBehavior {
class ApplicationContext {
private:
    // Service ownership (order matters for destruction)
    std::unique_ptr<ILogger> logger;
    std::unique_ptr<ITimeManager> time_manager;
    std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider;
    std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider;
    std::unique_ptr<IJsonLoader> json_loader;
    std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager;
    std::unique_ptr<IFrameworkSchemaManager> schema_manager;
    std::unique_ptr<FrameworkRegistry> registry;
    std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator;

    CoreServices core_services;
    ConfigurationServices configuration_services;
    DomainServices domain_services;
    RegistryServices registry_services;
    ApplicationServices application_services;

public:
    ApplicationContext(
        std::unique_ptr<ILogger> logger,
        std::unique_ptr<ITimeManager> time_manager,
        std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider,
        std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider,
        std::unique_ptr<IJsonLoader> json_loader,
        std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager,
        std::unique_ptr<IFrameworkSchemaManager> schema_manager,
        std::unique_ptr<FrameworkRegistry> registry,
        std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator) :
        logger(std::move(logger)),
        time_manager(std::move(time_manager)),
        environmental_condition_provider(std::move(environmental_condition_provider)),
        start_character_action_provider(std::move(start_character_action_provider)),
        json_loader(std::move(json_loader)),
        environmental_condition_manager(std::move(environmental_condition_manager)),
        schema_manager(std::move(schema_manager)),
        registry(std::move(registry)),
        state_operation_evaluator(std::move(state_operation_evaluator)),
        core_services(*this->logger, *this->time_manager, *this->environmental_condition_provider,
            *this->start_character_action_provider),
        configuration_services(core_services, *this->json_loader),
        domain_services(configuration_services, *this->environmental_condition_manager, *this->schema_manager),
        application_services(domain_services, *this->state_operation_evaluator),
        registry_services(application_services, *this->registry, *this->registry, *this->registry) {}

    CoreServices& Core() { return core_services; }
    ConfigurationServices& Configuration() { return configuration_services; }
    DomainServices& Domain() { return domain_services; }
    RegistryServices& Registry() { return registry_services; }
    ApplicationServices& Application() { return application_services; }
};
}
