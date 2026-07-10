#include "core.h"
#include "raylib.h"

void GameStateManager::PushState(std::unique_ptr<IGameState> state) {
    states.push(std::move(state));
    states.top()->Initialize();
}

void GameStateManager::PopState() {
    if (!states.empty()) {
        states.top()->Cleanup();
        states.pop();
    }
}

void GameStateManager::Update(float deltaTime) {
    if (!states.empty()) states.top()->Update(deltaTime);
}

void GameStateManager::Draw() {
    if (!states.empty()) states.top()->Draw();
}

void GameplayState::Initialize() {}
void GameplayState::Update(float deltaTime) {}
void GameplayState::Draw() {
    ClearBackground(SKYBLUE);
    DrawText("Gameplay - Coming Soon", 200, 300, 30, BLACK);
}
void GameplayState::Cleanup() {}
