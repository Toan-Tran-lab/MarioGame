#pragma once
#include "core/core.h"
#include "Button/Button.h"
#include "Level/Level.h"
#include <vector>

class LevelSelectState : public IGameState {
private:
    std::vector<Button> levelButtons;
    Button backButton;
    int selectedIndex = 0;
    float timeAccum = 0.0f;
    Vector2 lastMousePos = { -1.0f, -1.0f };

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
