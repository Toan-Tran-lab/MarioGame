#pragma once
#include "core/core.h"
#include "world/TileMap.h"
#include "Level/Level.h"
#include "physics/PhysicsBody.h"
#include "physics/InputManager.h"
#include "Camera/View.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/Player.h"
#include "Game Objects/Derived Objects/Enemies/Goomba/Goomba.h"

#include "SaveManager/SaveManager.h"
#include <vector>
#include <memory>

class GameplayState : public IGameState {
private:
    TileMap tileMap;
    View view;
    Level currentLevel;

    Goomba goomba_;

    std::unique_ptr<Player> player_;
    std::vector<Rectangle> mapCollisionRects;

    // HUD & Game Data
    int score = 0;
    float timeLeft = 300.0f; // 300 seconds default
    bool isGameOver = false;
    bool isGameWon = false;

public:
    GameplayState();
    
    void SetLevel(const Level& level);
    void SetCharacter(int characterId);
    // Use this to override initial data when loading from a save file
    void SetLoadedData(Vector2 pos, int loadedScore, float loadedTime);
    SaveData GetSaveData() const;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
