#pragma once
#include "core/core.h"
#include "world/TileMap.h"
#include "Level/Level.h"
#include "physics/PhysicsBody.h"
#include "physics/InputManager.h"
#include "Camera/View.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "Game_Objects/Derived_Objects/Enemies/Goomba/Goomba.h"
#include "Game_Objects/Derived_Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game_Objects/Derived_Objects/Enemies/BuzzyBeetle/BuzzyBeetle.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Block/Block.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Luckyblock/Luckyblock.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Debris/Debris.h"
#include "Game_Objects/Derived_Objects/Items/Coin/Coin.h"
#include "Game_Objects/Derived_Objects/Items/Mushroom/Mushroom.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Flagpole/Flagpole.h"
#include "Game_Objects/Derived_Objects/Static_Objects/FlyingBridge/FlyingBridge.h"
#include "Game_Objects/Derived_Objects/Static_Objects/GoalPipe/GoalPipe.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Fire/Fire.h"

#include "SaveManager/SaveManager.h"
#include <vector>
#include <memory>

class GameplayState : public IGameState {
private:
    TileMap tileMap;
    View view;
    Level currentLevel;

    std::vector<std::unique_ptr<Goomba>> goombas_;
    std::vector<std::unique_ptr<KoopaShell>> koopas_;
    std::vector<std::unique_ptr<BuzzyBeetle>> buzzyBeetles_;
    std::vector<std::unique_ptr<Coin>> coins_;
    std::vector<DebrisPiece> debrisList_;
    Flagpole flagpole_;
    std::vector<std::unique_ptr<FlyingBridge>> flyingBridges_;
    std::vector<std::unique_ptr<GoalPipe>> goalPipes_;
    std::vector<std::unique_ptr<Fire>> fires_;
    Mushroom mushroom_;

    std::unique_ptr<Player> player_;

    bool isSandboxMode_ = false;
    std::vector<std::vector<SandboxCellData>> sandboxGrid_;
    int characterId_ = 0;
    bool levelCompleteTriggered_ = false;
    bool levelCompletePushed_ = false;

    // HUD & Game Data
    int score = 0;
    float timeLeft = 300.0f; // 300 seconds default
    bool isGameOver = false;
    bool hasLoadedPosition_ = false; // Set true by SetLoadedData to skip spawn override

public:
    GameplayState();
    
    void SetLevel(const Level& level);
    void SetCharacter(int characterId);
    void SetSandboxMode(const std::vector<std::vector<SandboxCellData>>& grid);
    // Use this to override initial data when loading from a save file
    void SetLoadedData(Vector2 pos, int loadedScore, float loadedTime);
    // Resets HUD/game state so the next level starts fresh
    void ResetForNewLevel();
    SaveData GetSaveData() const;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
