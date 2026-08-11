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

void GameStateManager::Update(float deltaTime) {
    if (!states.empty()) states.top()->Update(deltaTime);
}

void GameStateManager::Draw() {
    if (!states.empty()) states.top()->Draw();
}

void GameplayState::Initialize() {
    player_.position = { 100, 300 };
    player_.size = { 32, 32 };

    enemy_.position = { 500, 400 };
    enemy_.size = { 32, 32 };

    blocks_ = {
        { 0, 500, 800, 100 },       // Floor
        { 200, 400, 100, 100 },     // Wall / Step
        { 400, 350, 100, 20 },      // Floating Platform
        { -50, 0, 50, 600 },        // Left boundary
        { 800, 0, 50, 600 }         // Right boundary
    };

    inputState_ = physics::InputState();
}

void GameplayState::Update(float deltaTime) {
    physics::InputManager::UpdateInput(inputState_);

    physics::PhysicsEngine::ApplyPhysics(player_, inputState_, deltaTime);

    physics::InputState enemyInput;
    physics::ProximityAI::UpdateAI(enemy_, player_, 300.0f, deltaTime, enemyInput, blocks_);

    physics::PhysicsEngine::ApplyPhysics(enemy_, enemyInput, deltaTime);

    physics::CollisionSystem::ResolveMapCollisions(player_, blocks_);
    physics::CollisionSystem::ResolveMapCollisions(enemy_, blocks_);

    physics::CollisionInfo pveCol = physics::CollisionSystem::GetCollisionInfo(player_.GetRect(), enemy_.GetRect());
    if (pveCol.side == physics::CollisionSide::BOTTOM) {
        enemy_.position.y = 1000;
        player_.velocity.y = physics::PhysicsEngine::JUMP_FORCE * 0.5f;
    } else if (pveCol.side == physics::CollisionSide::LEFT || pveCol.side == physics::CollisionSide::RIGHT || pveCol.side == physics::CollisionSide::TOP) {
        player_.position.x = 100;
        player_.position.y = 300;
    }
}

void GameplayState::Draw() {
    ClearBackground(RAYWHITE);

    for (const auto& block : blocks_) {
        DrawRectangleRec(block, DARKGRAY);
    }

    DrawRectangleRec(player_.GetRect(), BLUE);
    DrawRectangleRec(enemy_.GetRect(), RED);

    DrawCircleLines((int)enemy_.position.x + 16, (int)enemy_.position.y + 16, 300.0f, Fade(RED, 0.3f));

    DrawText("Physics Sandbox: Use Arrows/WASD to Move, Space/W to Jump, Shift/Z to Sprint.", 10, 10, 20, DARKGRAY);
}

void GameplayState::Cleanup() {}
