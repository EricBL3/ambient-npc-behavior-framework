#pragma once
#include "services/interfaces/ISelectionAlgorithmManager.h"
#include <gmock/gmock.h>

namespace AmbientCharacterBehavior {
class MockSelectionAlgorithmManager : public ISelectionAlgorithmManager {
    MOCK_METHOD(SelectionAlgorithmOption, GetSelectionAlgorithmOption, (), (const, override));
    MOCK_METHOD(void, SetSelectionAlgorithmOption, (SelectionAlgorithmOption new_value), (override));
};
}
