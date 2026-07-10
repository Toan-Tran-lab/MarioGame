#pragma once
#include "core/core.h"
#include "Button/Button.h"
#include "Dropdown/Dropdown.h"
#include <vector>
#include <string>

class SettingsState : public IGameState {
private:
    struct ResolutionOption {
        int width;
        int height;
        std::string label;
    };

    std::vector<ResolutionOption> resolutions;
    Dropdown resolutionDropdown;
    Button backButton;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
