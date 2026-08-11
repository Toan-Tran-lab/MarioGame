#include "core.h"
#include "raylib.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
#include "physics/InputManager.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "physics/ProximityAI.h"
#include <vector>

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

void GameStateManager::PopToMainMenu() {
    while (states.size() > 1) {
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

