#pragma once
#include "services/interfaces/ISelectionAlgorithmManager.h"

namespace AmbientCharacterBehavior {
class SelectionAlgorithmManager : public ISelectionAlgorithmManager {
private:
    SelectionAlgorithmOption selection_algorithm_option;

public:
    explicit SelectionAlgorithmManager()
    { selection_algorithm_option = SelectionAlgorithmOption::MEMORY_BASED; }

    ~SelectionAlgorithmManager() override = default;

    SelectionAlgorithmOption GetSelectionAlgorithmOption() const override
    { return selection_algorithm_option; }

    void SetSelectionAlgorithmOption(SelectionAlgorithmOption new_value) override
    {
        if (new_value != selection_algorithm_option)
        {
            selection_algorithm_option = new_value;
        }
    }
};
}
