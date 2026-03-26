#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include "behavior/Action.h"
#include "behavior/Sequence.h"

namespace AmbientCharacterBehavior {
class IContentProvider {
public:
    virtual ~IContentProvider() = default;

    virtual bool RegisterSequences(const std::string& config_file_path) = 0;
    virtual bool RegisterActions(const std::string& config_file_path) = 0;

    virtual bool HasSequence(int32_t sequence_id) const = 0;
    virtual std::shared_ptr<Sequence> GetSequenceById(int32_t sequence_id) const = 0;
    virtual bool HasAction(int32_t action_id) const = 0;
    virtual std::shared_ptr<Action> GetActionById(int32_t action_id) const = 0;
};
}
