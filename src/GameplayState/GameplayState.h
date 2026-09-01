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
#include "Game_Objects/Derived_Objects/Enemies/Piranha/Piranha.h"
#include "Game_Objects/Derived_Objects/Enemies/Boss/SpecificBoss/DragonBoss/DragonBoss.h"
#include "Game_Objects/Derived_Objects/Projectiles/Fireball/Fireball.h"
#include "Game_Objects/Derived_Objects/Enemies/Bullet/Bullet.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Princess/Princess.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Block/Block.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Luckyblock/Luckyblock.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Debris/Debris.h"
#include "Game_Objects/Derived_Objects/Static_Objects/GoalPipe/GoalPipe.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Flagpole/Flagpole.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Fire/Fire.h"
#include "Game_Objects/Derived_Objects/Items/Coin/Coin.h"
#include "Game_Objects/Derived_Objects/Items/Mushroom/Mushroom.h"
#include "Game_Objects/Derived_Objects/Items/FireFlower/FireFlower.h"
#include "Game_Objects/Derived_Objects/Items/Starman/Starman.h"
#include "Game_Objects/Derived_Objects/Projectiles/PlayerFireball/PlayerFireball.h"
#include "Game_Objects/Derived_Objects/Static_Objects/FlyingBridge/FlyingBridge.h"

#include "SaveManager/SaveManager.h"
#include <vector>
#include <memory>

struct BulletTrigger {
    float triggerX = 0.0f;
    float spawnY = 0.0f;
    float direction = 1.0f; // 1.0f = from left to right, -1.0f = from right to left
    bool triggered = false;
};

struct ScorePopup {
    Vector2 position;
    float timer;
    int score;
};

class GameplayState : public IGameState {
private:
    TileMap tileMap;
    View view;
    Level currentLevel;

    std::vector<ScorePopup> scorePopups_;
    std::vector<std::unique_ptr<Goomba>> goombas_;
    std::vector<std::unique_ptr<KoopaShell>> koopas_;
    std::vector<std::unique_ptr<BuzzyBeetle>> buzzyBeetles_;
    std::vector<std::unique_ptr<Piranha>> piranhas_;
    std::unique_ptr<DragonBoss> dragonBoss_;
    std::vector<std::unique_ptr<Fireball>> fireballs_;
    std::vector<std::unique_ptr<Bullet>> bullets_;
    std::vector<BulletTrigger> bulletTriggers_;
    std::vector<std::unique_ptr<Coin>> coins_;
    std::vector<DebrisPiece> debrisList_;
    std::unique_ptr<Princess> princess_;
    std::unique_ptr<GoalPipe> goalPipe_;
    std::unique_ptr<Flagpole> flagpole_;
    std::vector<std::unique_ptr<Mushroom>> mushrooms_;
    std::vector<std::unique_ptr<FireFlower>> fireFlowers_;
    std::vector<std::unique_ptr<Starman>> starmen_;
    std::vector<std::unique_ptr<PlayerFireball>> playerFireballs_;
    std::vector<std::unique_ptr<FlyingBridge>> flyingBridges_;
    std::vector<std::unique_ptr<Fire>> fires_;

    std::unique_ptr<Player> player_;
    int characterId_ = 0;

    // --- 2-Player Co-op ---
    std::unique_ptr<Player> player2_;
    int characterId2_ = 1; // P2 defaults to the opposite character (Luigi if P1 is Mario)
    bool isMultiplayer_ = false;

    bool isSandboxMode_ = false;
    std::vector<std::vector<SandboxCellData>> sandboxGrid_;

    float smoothedCamX_ = 0.0f;
    float smoothedCamY_ = 0.0f;
    bool firstCameraInit_ = true;
    Player* winningPlayer_ = nullptr;

    // HUD & Game Data
    int score = 0;
    float timeLeft = 300.0f; // 300 seconds default
    bool isGameOver = false;
    bool isGameWon = false;
    bool hasLoadedPosition_ = false;

public:
    GameplayState();
    
    void SetLevel(const Level& level);
    void SetCharacter(int characterId);
    void SetMultiplayer(bool enabled); // Enable 2-player co-op; P2 uses the opposite character
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
