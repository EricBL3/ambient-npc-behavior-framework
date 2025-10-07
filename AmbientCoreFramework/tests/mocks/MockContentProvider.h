#pragma once
#include <gmock/gmock.h>
#include "interfaces/IContentProvider.h"

namespace AmbientCharacterBehavior {
class MockContentProvider : public IContentProvider {
public:
    MOCK_METHOD(bool, RegisterSequences, (const std::string& config_file_path), (override));
    MOCK_METHOD(bool, RegisterActions, (const std::string& config_file_path), (override));

    MOCK_METHOD(bool, HasSequence, (int32_t sequence_id), (const, override));
    MOCK_METHOD(std::shared_ptr<Sequence>, GetSequenceById, (int32_t sequence_id), (const, override));
    MOCK_METHOD(bool, HasAction, (int32_t action_id), (const, override));
    MOCK_METHOD(std::shared_ptr<Action>, GetActionById, (int32_t action_id), (const, override));
};
}
