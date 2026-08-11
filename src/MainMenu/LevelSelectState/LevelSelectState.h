#pragma once
#include "core/core.h"
#include "Button/Button.h"
#include "Level/Level.h"
#include <vector>

class LevelSelectState : public IGameState {
private:
    std::vector<Button> levelButtons;
    Button backButton;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
