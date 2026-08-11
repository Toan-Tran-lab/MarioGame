#pragma once
#include "core/core.h"
#include "world/TileMap.h"
#include "Level/Level.h"
#include "physics/PhysicsBody.h"
#include "physics/InputManager.h"
#include "Camera/GameCamera.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/Player.h"

class GameplayState : public IGameState {
private:
    TileMap tileMap;
    GameCamera camera;
    Level currentLevel;

    Player player_;
    physics::PhysicsBody enemy_; // Keeping the enemy for now, but will make it spawn differently later
    std::vector<Rectangle> mapCollisionRects;

public:
    GameplayState();
    
    void SetLevel(const Level& level);
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
