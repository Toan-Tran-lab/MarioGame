#pragma once
#include "core/core.h"
#include "world/TileMap.h"
#include "Level/Level.h"
#include "physics/PhysicsBody.h"
#include "physics/InputManager.h"
#include "Camera/View.h"
<<<<<<< HEAD
#include "Game Objects/Derived Objects/Playable Characters/Player/Player.h"
#include "Game Objects/Derived Objects/Enemies/Goomba/Goomba.h"
<<<<<<< HEAD
=======
#include "Game Objects/Derived Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game Objects/Derived Objects/Static Objects/Coin/Coin.h"
#include "Game Objects/Derived Objects/Static Objects/Block.h"
#include "Game Objects/Derived Objects/Static Objects/Luckyblock/Luckyblock.h"
#include "Game Objects/Derived Objects/Static Objects/Debris.h"
#include "Game Objects/Derived Objects/Items/Mushroom.h"
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c
=======
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "Game_Objects/Derived_Objects/Enemies/Goomba/Goomba.h"
#include "Game_Objects/Derived_Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game_Objects/Derived_Objects/Enemies/BuzzyBeetle/BuzzyBeetle.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Block/Block.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Luckyblock/Luckyblock.h"
#include "Game_Objects/Derived_Objects/Static_Objects/Debris/Debris.h"
#include "Game_Objects/Derived_Objects/Items/Coin/Coin.h"
#include "Game_Objects/Derived_Objects/Items/Mushroom/Mushroom.h"
>>>>>>> eb909805f952015eba6442d544c72d2a1154d5eb

#include "SaveManager/SaveManager.h"
#include <vector>
#include <memory>

class GameplayState : public IGameState {
private:
    TileMap tileMap;
    View view;
    Level currentLevel;

<<<<<<< HEAD
    Goomba goomba_;
=======
    std::vector<std::unique_ptr<Goomba>> goombas_;
    std::vector<std::unique_ptr<KoopaShell>> koopas_;
    std::vector<std::unique_ptr<BuzzyBeetle>> buzzyBeetles_;
    std::vector<std::unique_ptr<Coin>> coins_;
    std::vector<DebrisPiece> debrisList_;
    Mushroom mushroom_;
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c

    std::unique_ptr<Player> player_;

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
