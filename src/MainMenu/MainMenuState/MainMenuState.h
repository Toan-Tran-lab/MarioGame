#pragma once
#include "core/core.h"
#include "Button/Button.h"
#include <vector>

class MainMenuState : public IGameState {
public:
    enum MenuItem {
        NEW_GAME = 0,
        SETTINGS,
        EXIT,
        BUTTON_COUNT
    };

private:
    std::vector<Button> buttons;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
