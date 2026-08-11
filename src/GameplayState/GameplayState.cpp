#include "GameplayState.h"
#include "raylib.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "physics/ProximityAI.h"
#include <iostream>

GameplayState::GameplayState() {
    currentLevel = Level::GetLevel(1);
}

void GameplayState::SetLevel(const Level& level) {
    currentLevel = level;
}

void GameplayState::Initialize() {
    // Load the tilemap based on current level
    std::string ldtkPath = "assets/maps/maps.ldtk";
    if (!tileMap.LoadFromLdtk(ldtkPath, currentLevel.GetLdtkLevelId())) {
        std::cerr << "Failed to load tilemap for level: " << currentLevel.GetLdtkLevelId() << std::endl;
    }

    // Load player animations/textures
    TextureManager::Load("mario_pose", "assets/textures/Mario/pose/mario.png");
    TextureManager::Load("mario_walk", "assets/textures/Mario/walk/mario.png");
    TextureManager::Load("mario_jump", "assets/textures/Mario/jump/mario.png");
    TextureManager::Load("mario_slide", "assets/textures/Mario/slide/mario.png");

    // Initialize player
    Vector2 spawn = tileMap.GetPlayerSpawn();
    Vector2 spawnPos = { spawn.x > 0 ? spawn.x : 100, spawn.y > 0 ? spawn.y : 300 };
    player_.SetPosition(spawnPos);
    player_.SetSize({ 48, 60 }); // Use SetSize so it persists into physics body
    player_.SyncPhysicsBody();
    
    // Extract map collision rects and pass to player
    mapCollisionRects = tileMap.GetCollisionRects();
    player_.SetCollisionBlocks(&mapCollisionRects);

    // Initialize enemy (placeholder)
    enemy_.position = { 500, 400 };
    enemy_.size = { 32, 32 };

    // Initialize camera
    camera.Init((float)tileMap.GetPixelWidth(), (float)tileMap.GetPixelHeight());
}

void GameplayState::Update(float deltaTime) {
    player_.Update(deltaTime);

    physics::InputState enemyInput;
    // For now, no proximity AI update since it relies on blocks_. We will fix ProximityAI later.
    // physics::ProximityAI::UpdateAI(enemy_, player_.GetPhysicsBody(), 300.0f, deltaTime, enemyInput, blocks_);
    physics::PhysicsEngine::ApplyPhysics(enemy_, enemyInput, deltaTime);

    // Grid-based collision for enemy (Now using rect list)
    physics::CollisionSystem::ResolveMapCollisions(enemy_, mapCollisionRects);

    // Entity collision
    physics::CollisionInfo pveCol = physics::CollisionSystem::GetCollisionInfo(player_.GetRect(), enemy_.GetRect());
    if (pveCol.side == physics::CollisionSide::BOTTOM) {
        enemy_.position.y = 1000;
        player_.GetPhysicsBody().velocity.y = physics::PhysicsEngine::JUMP_FORCE * 0.6f;
        player_.SyncPhysics(); // Sync changes back to player
    } else if (pveCol.side == physics::CollisionSide::LEFT || pveCol.side == physics::CollisionSide::RIGHT || pveCol.side == physics::CollisionSide::TOP) {
        // Player dies / resets
        Vector2 spawn = tileMap.GetPlayerSpawn();
        Vector2 spawnPos = { spawn.x > 0 ? spawn.x : 100, spawn.y > 0 ? spawn.y : 300 };
        player_.SetPosition(spawnPos);
        player_.SyncPhysicsBody();
    }

    camera.Update(player_.GetPosition().x, player_.GetPosition().y);
}

void GameplayState::Draw() {
    ClearBackground(tileMap.GetBackgroundColor());

    camera.BeginDraw();

    tileMap.Draw(camera.GetWorldLeft(), camera.GetWorldTop(), camera.GetRawCamera().zoom);

    player_.Draw();
    DrawRectangleRec(enemy_.GetRect(), RED);

    camera.EndDraw();

    DrawText("Physics Sandbox: Use Arrows/WASD to Move, Space/W to Jump, Shift/Z to Sprint.", 10, 10, 20, DARKGRAY);
}

void GameplayState::Cleanup() {}
