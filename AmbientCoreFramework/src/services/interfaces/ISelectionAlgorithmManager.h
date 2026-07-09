#pragma once
#include "services/layers/0_foundation/SelectionAlgorithmOption.h"

namespace AmbientCharacterBehavior {

class ISelectionAlgorithmManager {
public:
    virtual ~ISelectionAlgorithmManager() = default;
    virtual SelectionAlgorithmOption GetSelectionAlgorithmOption() const = 0;
    virtual void SetSelectionAlgorithmOption(SelectionAlgorithmOption new_value) = 0;
};

}
