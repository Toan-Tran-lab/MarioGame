#include "GameplayState.h"
#include "raylib.h"
#include "TextureManager/TextureManager.h"
#include "AudioManager/AudioManager.h"
#include "Global/Global.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "physics/ProximityAI.h"
#include "SaveManager/SaveManager.h"
#include "MainMenu/PauseMenuState/PauseMenuState.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Specific/Mario/Mario.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Specific/Luigi/Luigi.h"
#include "GameplayState/LevelCompleteState/LevelCompleteState.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace {
bool RectOverlapsSolidBlock(const Rectangle& rect, const BlockGrid& grid) {
    int tileSize = grid.GetTileSize();
    if (tileSize <= 0) return false;

    int startCol = (int)(rect.x / tileSize);
    int endCol   = (int)((rect.x + rect.width) / tileSize);
    int startRow = (int)(rect.y / tileSize);
    int endRow   = (int)((rect.y + rect.height) / tileSize);

    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (grid.IsSolidAt(col, row)) return true;
        }
    }
    return false;
}
}

GameplayState::GameplayState() {
    currentLevel = Level::GetLevel(1);
    player_ = std::make_unique<Mario>();
}

void GameplayState::SetLevel(const Level& level) {
    currentLevel = level;
}

void GameplayState::SetCharacter(int characterId) {
    characterId_ = characterId;
    if (characterId == 1) {
        player_ = std::make_unique<Luigi>();
    } else {
        player_ = std::make_unique<Mario>();
    }
}

void GameplayState::SetMultiplayer(bool enabled) {
    isMultiplayer_ = enabled;
    if (enabled) {
        // P2 is always the other character
        characterId2_ = (characterId_ == 0) ? 1 : 0;
        if (characterId2_ == 1) {
            player2_ = std::make_unique<Luigi>();
        } else {
            player2_ = std::make_unique<Mario>();
        }

        // P1 key bindings: WASD + Left Shift + F
        physics::PlayerKeyBindings p1Bindings;
        p1Bindings.left    = KEY_A;
        p1Bindings.right   = KEY_D;
        p1Bindings.down    = KEY_S;
        p1Bindings.jump    = KEY_W;
        p1Bindings.jumpAlt = KEY_SPACE;
        p1Bindings.sprint  = KEY_LEFT_SHIFT;
        p1Bindings.shoot   = KEY_F;
        player_->SetKeyBindings(p1Bindings);

        // P2 key bindings: Arrow keys + Right Shift + J
        physics::PlayerKeyBindings p2Bindings;
        p2Bindings.left    = KEY_LEFT;
        p2Bindings.right   = KEY_RIGHT;
        p2Bindings.down    = KEY_DOWN;
        p2Bindings.jump    = KEY_UP;
        p2Bindings.jumpAlt = 0; // no alt key
        p2Bindings.sprint  = KEY_RIGHT_SHIFT;
        p2Bindings.shoot   = KEY_J;
        player2_->SetKeyBindings(p2Bindings);
    } else {
        player2_.reset();
    }
}

void GameplayState::SetSandboxMode(const std::vector<std::vector<SandboxCellData>>& grid) {
    isSandboxMode_ = true;
    sandboxGrid_ = grid;
}

void GameplayState::ResetForNewLevel() {
    // Reset timer and state flags
    timeLeft = 300.0f;
    isGameOver = false;
    isGameWon = false;
    firstCameraInit_ = true;
    winningPlayer_ = nullptr;

    // Reset player's position to (0,0) so that Initialize() knows to pick up the new level's spawn point
    if (player_) {
        player_->SetPosition({0, 0});
        
        // Also reset physics properties that might cause weird behavior on a new level
        player_->GetPhysicsBody().velocity = {0, 0};
        player_->GetPhysicsBody().isGrounded = false;
        player_->GetPhysicsBody().hitCeiling = false;
    }
}

void GameplayState::SetLoadedData(Vector2 pos, int loadedScore, float loadedTime) {
    player_->SetPosition(pos);
    score = loadedScore;
    timeLeft = loadedTime;
    // We can use a flag or just rely on position.
    // If the position is not (0,0), we might skip spawn overriding in Initialize.
}

SaveData GameplayState::GetSaveData() const {
    SaveData data;
    data.levelId = currentLevel.GetLevelNumber();
    data.playerX = player_->GetPosition().x;
    data.playerY = player_->GetPosition().y;
    data.score = score;
    data.timeLeft = timeLeft;

    data.isSandboxMode = isSandboxMode_;
    if (isSandboxMode_) {
        data.sandboxGrid = sandboxGrid_;
    }
    return data;
}

void GameplayState::Initialize() {
    firstCameraInit_ = true;
    winningPlayer_ = nullptr;
    
    // Load the tilemap based on sandbox mode or level selection
    if (isSandboxMode_) {
        if (!tileMap.LoadFromSandbox(sandboxGrid_)) {
            std::cerr << "Failed to load custom sandbox map" << std::endl;
        }
    } else {
        std::string ldtkPath = "assets/maps/maps.ldtk";
        if (!tileMap.LoadFromLdtk(ldtkPath, currentLevel.GetLdtkLevelId())) {
            std::cerr << "Failed to load tilemap for level: " << currentLevel.GetLdtkLevelId() << std::endl;
        }
    }

    // Load player animations/textures
    // Mario Normal
    TextureManager::Load("mario_pose", "assets/textures/Mario/pose/mario.png");
    TextureManager::Load("mario_walk", "assets/textures/Mario/walk/mario.png");
    TextureManager::Load("mario_jump", "assets/textures/Mario/jump/mario.png");
    TextureManager::Load("mario_slide", "assets/textures/Mario/slide/mario.png");
    TextureManager::Load("mario_sit", "assets/textures/Mario/sit/mario.png");

    // Mario Mini
    TextureManager::Load("mario_mini_pose", "assets/textures/Mario-mini/pose/mario.png");
    TextureManager::Load("mario_mini_walk", "assets/textures/Mario-mini/walk/mario.png");
    TextureManager::Load("mario_mini_jump", "assets/textures/Mario-mini/jump/mario.png");
    TextureManager::Load("mario_mini_slide", "assets/textures/Mario-mini/slide/mario.png");
    TextureManager::Load("mario_mini_die", "assets/textures/Mario-mini/die/mario.png");
    TextureManager::Load("mario_mini_sit", "assets/textures/Mario/sit/mario.png"); // using normal sit if mini sit isn't provided

    // Fire Mario
    TextureManager::Load("fire_mario_pose", "assets/textures/Fire Mario/pose/fireMario.png");
    TextureManager::Load("fire_mario_walk", "assets/textures/Fire Mario/walk/fireMario.png");
    TextureManager::Load("fire_mario_jump", "assets/textures/Fire Mario/jump/fireMario.png");
    TextureManager::Load("fire_mario_slide", "assets/textures/Fire Mario/slide/fireMario.png");
    TextureManager::Load("fire_mario_sit", "assets/textures/Fire Mario/sit/fireMario.png");
    TextureManager::Load("fire_mario_shoot", "assets/textures/Fire Mario/shoot/fireMario.png");

    // Luigi Normal
    TextureManager::Load("luigi_pose", "assets/textures/Luigi/pose/luigi.png");
    TextureManager::Load("luigi_walk", "assets/textures/Luigi/walk/luigi.png");
    TextureManager::Load("luigi_jump", "assets/textures/Luigi/jump/luigi.png");
    TextureManager::Load("luigi_slide", "assets/textures/Luigi/slide/luigi.png");
    TextureManager::Load("luigi_sit", "assets/textures/Luigi/sit/luigi.png");
    TextureManager::Load("luigi_die", "assets/textures/Luigi/die/luigi.png");
    
    // Luigi Mini
    TextureManager::Load("luigi_mini_pose", "assets/textures/Luigi-mini/pose/luigi.png");
    TextureManager::Load("luigi_mini_walk", "assets/textures/Luigi-mini/walk/luigi.png");
    TextureManager::Load("luigi_mini_jump", "assets/textures/Luigi-mini/jump/luigi.png");
    TextureManager::Load("luigi_mini_slide", "assets/textures/Luigi-mini/slide/luigi.png");
    TextureManager::Load("luigi_mini_die", "assets/textures/Luigi-mini/die/luigi.png");
    TextureManager::Load("luigi_mini_sit", "assets/textures/Luigi/sit/luigi.png"); // fallback

    // Fire Luigi
    TextureManager::Load("fire_luigi_pose", "assets/textures/Fire Luigi/pose/fireLuigi.png");
    TextureManager::Load("fire_luigi_walk", "assets/textures/Fire Luigi/walk/fireLuigi.png");
    TextureManager::Load("fire_luigi_jump", "assets/textures/Fire Luigi/jump/fireLuigi.png");
    TextureManager::Load("fire_luigi_slide", "assets/textures/Fire Luigi/slide/fireLuigi.png");
    TextureManager::Load("fire_luigi_sit", "assets/textures/Fire Luigi/sit/fireLuigi.png");
    TextureManager::Load("fire_luigi_shoot", "assets/textures/Fire Luigi/shoot/fireLuigi.png");

    // Items
    TextureManager::Load("mushroom", "assets/textures/mushroom/mushroom.png");

    // UI
    TextureManager::Load("gameover_img", "assets/textures/gameover.png");

    // Load Goomba, Koopa and Coin textures
    TextureManager::Load("goomba_texture", "assets/textures/enemies-3.png"); // legacy (sandbox preview)
    TextureManager::Load("goomba_walk",    "assets/textures/Goomba/walk/enemies.png");
    TextureManager::Load("goomba_dead",    "assets/textures/Goomba/dead/enemies.png");
    TextureManager::Load("koopa_walk",     "assets/textures/Koopa/walk/enemies.png");
    TextureManager::Load("koopa_hide",     "assets/textures/Koopa/hide/enemies.png");
    TextureManager::Load("buzzy_walk",    "assets/textures/BuzzyBeetle/walk/enemies.png");
    TextureManager::Load("buzzy_hide",    "assets/textures/BuzzyBeetle/hide/enemies.png");
    TextureManager::Load("piranha",       "assets/textures/Piranha/enemies.png");
    TextureManager::Load("bullet",        "assets/textures/Bullet/enemies.png");
    TextureManager::Load("coin", "assets/textures/coin/coin.png");
    TextureManager::Load("mushroom", "assets/textures/Items/items.png");
    TextureManager::Load("luckyblock", "assets/textures/Luckyblock/luckyblock.png");

    // Initialize player
    if (player_->GetPosition().x == 0 && player_->GetPosition().y == 0) {
        Vector2 spawn = tileMap.GetPlayerSpawn();
        Vector2 spawnPos = { spawn.x > 0 ? spawn.x : 100, spawn.y > 0 ? spawn.y : 300 };
        player_->SetPosition(spawnPos);
    }
    if (player_->IsSmall()) {
        player_->SetSize({Global::MINI_PLAYER_WIDTH * Global::GAME_SCALE, Global::MINI_PLAYER_HEIGHT * Global::GAME_SCALE}); 
    } else {
        player_->SetSize({Global::SUPER_PLAYER_WIDTH * Global::GAME_SCALE, Global::SUPER_PLAYER_HEIGHT * Global::GAME_SCALE}); 
    }
    player_->SyncPhysicsBody();

    // Initialize Player 2 (multiplayer only)
    if (isMultiplayer_ && player2_) {
        Vector2 p1Pos = player_->GetPosition();
        // Spawn P2 one tile to the right of P1 so they don't overlap
        player2_->SetPosition({ p1Pos.x + Global::TILE_SIZE * Global::GAME_SCALE * 2.0f, p1Pos.y });
        player2_->SetSize(player_->GetSize()); // match P1's current hitbox size
        player2_->SyncPhysicsBody();
        player2_->SetCollisionGrid(&tileMap.GetBlockGrid());
    }

    // Initialize enemies and coins
    goombas_.clear();
    koopas_.clear();
    buzzyBeetles_.clear();
    piranhas_.clear();
    bullets_.clear();
    bulletTriggers_.clear();
    dragonBoss_.reset();
    dragonFlames_.clear();
    shockwaveManager_.Clear();
    fireballs_.clear();
    coins_.clear();
    princess_.reset();
    goalPipe_.reset();
    flagpole_.reset();
    mushrooms_.clear();
    fireFlowers_.clear();
    starmen_.clear();
    playerFireballs_.clear();
    flyingBridges_.clear();
    fires_.clear();
    mapDoorBlocks_.clear();

    bossBattleCtrl_.Reset();

    if (isSandboxMode_) {
        // Spawn Goombas and Coins based on Sandbox coordinates
        for (int r = 0; r < tileMap.GetMapHeight(); ++r) {
            for (int c = 0; c < tileMap.GetMapWidth(); ++c) {
                if (c >= (int)sandboxGrid_[r].size()) continue;
                const auto& cell = sandboxGrid_[r][c];
                if (cell.type == 3) {
                    auto g = std::make_unique<Goomba>();
                    g->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) });
                    g->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                    g->SetPlayerBody(&player_->GetPhysicsBody());
                    g->SetCollisionGrid(&tileMap.GetBlockGrid());
                    goombas_.push_back(std::move(g));
                } else if (cell.type == 4) {
                    auto c_coin = std::make_unique<Coin>();
                    c_coin->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) });
                    c_coin->SetSize({ 48.0f, 48.0f });
                    coins_.push_back(std::move(c_coin));
                } else if (cell.type == 5) {
                    auto k = std::make_unique<KoopaShell>();
                    // Normal KoopaShell size is 16x24, same adjustment as the normal-level spawn path.
                    k->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) - 8.0f * Global::GAME_SCALE });
                    k->SetSize({ 16.0f * Global::GAME_SCALE, 24.0f * Global::GAME_SCALE });
                    k->SetPlayerBody(&player_->GetPhysicsBody());
                    k->SetCollisionGrid(&tileMap.GetBlockGrid());
                    koopas_.push_back(std::move(k));
                } else if (cell.type == 6) {
                    auto b = std::make_unique<BuzzyBeetle>();
                    b->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) });
                    b->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                    b->SetPlayerBody(&player_->GetPhysicsBody());
                    b->SetCollisionGrid(&tileMap.GetBlockGrid());
                    buzzyBeetles_.push_back(std::move(b));
                }
            }
        }
    } else {
        // Normal level entity spawn from TileMap
        auto ToLowerStr = [](std::string s) {
            for (char& c : s) c = (char)std::tolower((unsigned char)c);
            return s;
        };

        const auto& entities = tileMap.GetEntities();
        for (const auto& ent : entities) {
            std::string id = ToLowerStr(ent.id);

            if (id == "goomba") {
                auto g = std::make_unique<Goomba>();
                g->SetPosition(ent.position);
                g->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                g->SetPlayerBody(&player_->GetPhysicsBody());
                g->SetCollisionGrid(&tileMap.GetBlockGrid());
                goombas_.push_back(std::move(g));
            } else if (id == "koopashell" || id == "koopa") {
                auto k = std::make_unique<KoopaShell>();
                k->SetPosition({ ent.position.x, ent.position.y - 8.0f * Global::GAME_SCALE });
                k->SetSize({ 16.0f * Global::GAME_SCALE, 24.0f * Global::GAME_SCALE });
                k->SetPlayerBody(&player_->GetPhysicsBody());
                k->SetCollisionGrid(&tileMap.GetBlockGrid());
                koopas_.push_back(std::move(k));
            } else if (id == "buzzybeetle" || id == "buzzy") {
                auto b = std::make_unique<BuzzyBeetle>();
                b->SetPosition(ent.position);
                b->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                b->SetPlayerBody(&player_->GetPhysicsBody());
                b->SetCollisionGrid(&tileMap.GetBlockGrid());
                buzzyBeetles_.push_back(std::move(b));
            } else if (id == "piranha" || id == "piranhaplant") {
                auto p = std::make_unique<Piranha>();
                Vector2 spawnPos = { ent.position.x + (float)tileMap.GetTileSize() / 2.0f, ent.position.y + 25 - tileMap.GetTileSize()};
                p->SetPosition(spawnPos);
                p->SetPlayerBody(&player_->GetPhysicsBody());
                piranhas_.push_back(std::move(p));
            } else if (id == "coin") {
                auto c_coin = std::make_unique<Coin>();
                c_coin->SetPosition(ent.position);
                c_coin->SetSize({ 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE });
                coins_.push_back(std::move(c_coin));
            } else if (id == "luckyblock") {
                auto block = std::make_unique<Luckyblock>();
                block->SetPosition(ent.position);
                int col = (int)(ent.position.x / tileMap.GetTileSize());
                int row = (int)(ent.position.y / tileMap.GetTileSize());
                tileMap.GetBlockGrid().SetBlock(col, row, std::move(block));
            } else if (id == "dragonboss" || id == "dragon_boss" || id == "boss") {
                dragonBoss_ = std::make_unique<DragonBoss>();
                dragonBoss_->SetPosition(ent.position);
                dragonBoss_->SetGroundY(ent.position.y);
                dragonBoss_->SetPlayerRef(player_.get());
                dragonBoss_->SetCollisionGrid(&tileMap.GetBlockGrid());
                dragonBoss_->BeginSpawn();
            } else if (id == "princess") {
                princess_ = std::make_unique<Princess>();
                princess_->SetPosition(ent.position);
                princess_->SetSize({ 16.0f * Global::GAME_SCALE, 32.0f * Global::GAME_SCALE }); // tune to art
            } else if (id == "flyingbridge") {
                auto bridge = std::make_unique<FlyingBridge>();
                bridge->SetPosition(ent.position);
                bridge->SetPatrolBounds(0.0f, (float)tileMap.GetPixelWidth());
                bridge->SetBlockGrid(&tileMap.GetBlockGrid());
                flyingBridges_.push_back(std::move(bridge));
            } else if (id == "bulletleft" || id == "bullet_left") {
                bulletTriggers_.push_back({ ent.position.x, ent.position.y, -1.0f, false });
            } else if (id == "bulletright" || id == "bullet_right") {
                bulletTriggers_.push_back({ ent.position.x, ent.position.y, 1.0f, false });
            } else if (id == "goalpipe") {
                goalPipe_ = std::make_unique<GoalPipe>();
                goalPipe_->SetPosition(ent.position);
            } else if (id == "flagpole") {
                if (!flagpole_) flagpole_ = std::make_unique<Flagpole>();
                flagpole_->AddPoleSegment(ent.position);
            } else if (id == "flag") {
                if (!flagpole_) flagpole_ = std::make_unique<Flagpole>();
                flagpole_->SetFlagPosition(ent.position);
            } else if (id == "fire") {
                auto fire = std::make_unique<Fire>();
                fire->SetPosition(ent.position);
                fires_.push_back(std::move(fire));
            } else if (id == "door" || id == "bossdoor" || id == "boss_door" || id == "redoor" || id == "re_door") {
                float tileSize = (float)tileMap.GetTileSize();
                mapDoorBlocks_.push_back(Rectangle{ ent.position.x, ent.position.y, tileSize, tileSize });
            } else if (id == "startbattle" || id == "start_battle") {
                bossBattleCtrl_.SetStartBattle(ent.position);
            }
        }
        
        if (flagpole_) {
            flagpole_->Finalize();
        }

        // Configure Boss Room Door bounds from map Door blocks
        if (!mapDoorBlocks_.empty()) {
            float maxX = mapDoorBlocks_[0].x;
            Vector2 maxPos = { mapDoorBlocks_[0].x, mapDoorBlocks_[0].y };
            for (const auto& r : mapDoorBlocks_) {
                if (r.x > maxX) { maxX = r.x; maxPos = { r.x, r.y }; }
            }
            bossBattleCtrl_.SetDoor(maxPos);
        }
    }

    // Now pass the block grid to the player
    player_->SetCollisionGrid(&tileMap.GetBlockGrid());

    // Initialize camera
    view = View(16.0f, (float)tileMap.GetTileSize());
    view.Init((float)tileMap.GetPixelWidth(), (float)tileMap.GetPixelHeight());
    view.SetAllowBackwardScroll(isSandboxMode_ || currentLevel.GetLevelNumber() == 3);

    // Auto-save logic
    if (!isSandboxMode_) {
        SaveData data;
        data.levelId = currentLevel.GetLevelNumber();
        data.playerX = player_->GetPosition().x;
        data.playerY = player_->GetPosition().y;
        data.score = score;
        data.timeLeft = timeLeft;
        SaveManager::SaveGame("auto_save", data);
        Global::hasSaveGame = true;
    }

    std::vector<Player*> activePlayers;
    if (player_) activePlayers.push_back(player_.get());
    if (isMultiplayer_ && player2_) activePlayers.push_back(player2_.get());

    if (dragonBoss_) {
        bossBattleCtrl_.BeginIntro(activePlayers);
        Vector2 bPos = dragonBoss_->GetPosition();
        smoothedCamX_ = bPos.x + dragonBoss_->GetSize().x * 0.5f;
        smoothedCamY_ = bPos.y + dragonBoss_->GetSize().y * 0.45f;
        view.Update(smoothedCamX_, smoothedCamY_, 1.6f);
    } else {
        smoothedCamX_ = player_->GetPosition().x;
        smoothedCamY_ = player_->GetPosition().y;
        view.Update(smoothedCamX_, smoothedCamY_, 1.0f);
    }
    firstCameraInit_ = false;
}

void GameplayState::Update(float deltaTime) {
    // Check Pause (ESC)
    if (IsKeyPressed(Global::keys.pause)) {
        Global::gameStateManager->PushState(std::make_unique<PauseMenuState>(this));
    }

    if (isGameOver || isGameWon) {
        if (IsKeyPressed(KEY_ENTER)) {
            Global::gameStateManager->PopState(); // Or go to Game Over screen

        }
        return;
    }

    timeLeft -= deltaTime;
    if (timeLeft <= 0) {
        timeLeft = 0;
        isGameOver = true;
        AudioManager::StopBGM();
    }

    // Monitor starman invincibility and audio
    bool anyStarActive = (player_ && player_->IsInvincible() && !player_->IsDead()) ||
                         (player2_ && player2_->IsInvincible() && !player2_->IsDead());
    if (AudioManager::IsStarmanBGMPlaying() && !anyStarActive) {
        AudioManager::StopStarmanBGM();
    }

    // Move bridges first so their position is current when Player::Update() runs.
    for (auto& bridge : flyingBridges_) {
        float oldX = bridge->GetPosition().x;
        bridge->Update(deltaTime);
        float deltaX = bridge->GetPosition().x - oldX;

        auto CheckAndCarry = [&](Player* p) {
            if (!p || p->IsDead()) return;
            Rectangle pRect = p->GetPhysicsBody().GetRect();
            Rectangle bRect = bridge->GetRect();
            float pBottom = pRect.y + pRect.height;
            float bTop    = bRect.y;
            bool hOverlap = (pRect.x + pRect.width > bRect.x) && (pRect.x < bRect.x + bRect.width);
            bool onTop = (pBottom >= bTop - 4.0f) && (pBottom <= bTop + 4.0f);
            
            if (hOverlap && onTop && deltaX != 0.0f) {
                Vector2 pos = p->GetPosition();
                pos.x += deltaX;
                p->SetPosition(pos);
                p->SyncPhysicsBody();
            }
        };

        CheckAndCarry(player_.get());
        if (isMultiplayer_) CheckAndCarry(player2_.get());
    }

    // Move bullets and carry players standing on them like FlyingBridge
    for (auto& bullet : bullets_) {
        if (!bullet->IsActive()) continue;
        float oldX = bullet->GetPosition().x;
        bullet->Update(deltaTime);
        float deltaX = bullet->GetPosition().x - oldX;

        auto CheckAndCarryBullet = [&](Player* p) {
            if (!p || p->IsDead()) return;
            Rectangle pRect = p->GetPhysicsBody().GetRect();
            Rectangle bRect = bullet->GetRect();
            float pBottom = pRect.y + pRect.height;
            float bTop    = bRect.y;
            bool hOverlap = (pRect.x + pRect.width > bRect.x) && (pRect.x < bRect.x + bRect.width);
            bool onTop = (pBottom >= bTop - 8.0f) && (pBottom <= bTop + 8.0f);
            
            if (hOverlap && onTop && deltaX != 0.0f) {
                Vector2 pos = p->GetPosition();
                pos.x += deltaX;
                p->SetPosition(pos);
                p->SyncPhysicsBody();
            }
        };

        CheckAndCarryBullet(player_.get());
        if (isMultiplayer_) CheckAndCarryBullet(player2_.get());
    }

    // Inject all bridge, bullet, and boss door rects as dynamic solid platforms
    {
        std::vector<Rectangle> platformRects;
        platformRects.reserve(flyingBridges_.size() + bullets_.size() + 2);
        for (auto& bridge : flyingBridges_) {
            platformRects.push_back(bridge->GetRect());
        }
        for (auto& bullet : bullets_) {
            if (bullet->IsActive()) {
                platformRects.push_back(bullet->GetRect());
            }
        }

        // Add map Door blocks into dynamic solid platforms (Exit door opens on RoomCleared)
        for (const auto& dRect : mapDoorBlocks_) {
            bool isExitDoor = (bossBattleCtrl_.HasDoor() && dRect.x >= bossBattleCtrl_.GetDoorPos().x - 10.0f);
            bool isOpen = isExitDoor && (bossBattleCtrl_.GetPhase() == BossBattlePhase::RoomCleared);
            if (!isOpen) {
                platformRects.push_back(dRect);
            }
        }

        // Add reDoor and Door solid collision boxes based on Boss Battle State
        bossBattleCtrl_.AppendDynamicBarriers(platformRects, dragonBoss_.get());

        player_->SetDynamicPlatforms(platformRects);
        if (isMultiplayer_ && player2_) {
            player2_->SetDynamicPlatforms(platformRects);
        }
    }

    player_->Update(deltaTime);
    if (bossBattleCtrl_.GetPhase() == BossBattlePhase::Intro) {
        player_->GetPhysicsBody().velocity.x = 0.0f;
        if (isMultiplayer_ && player2_) {
            player2_->GetPhysicsBody().velocity.x = 0.0f;
        }
    }

    // Screen bounds constraints
    float cameraLeft = view.GetWorldLeft();
    float cameraWidth = (float)GetScreenWidth() / view.GetRawCamera().zoom;
    float cameraRight = cameraLeft + cameraWidth;

    auto ConstrainPlayerToScreen = [&](Player* p) {
        if (!p || p->IsDead()) return;
        
        float px = p->GetPosition().x;
        float pWidth = p->GetSize().x;
        
        // Left constraint: prevents moving beyond left edge of the map
        if (px < 0.0f) {
            p->SetPosition({0.0f, p->GetPosition().y});
            p->SyncPhysicsBody();
            if (p->GetPhysicsBody().velocity.x < 0) {
                p->GetPhysicsBody().velocity.x = 0.0f;
            }
        }
    };

    if (!player_->IsDead()) {
        ConstrainPlayerToScreen(player_.get());
    }

    // Update P2 (multiplayer only)
    if (isMultiplayer_ && player2_) {
        player2_->Update(deltaTime);
        if (!player2_->IsDead()) {
            ConstrainPlayerToScreen(player2_.get());
        }
    }



    // Dynamic AI Targeting: Find the closest alive player for enemies
    auto GetTargetPlayer = [&](const Vector2& enemyPos) -> Player* {
        bool p1Alive = player_ && !player_->IsDead();
        bool p2Alive = isMultiplayer_ && player2_ && !player2_->IsDead();
        
        if (p1Alive && p2Alive) {
            float d1 = std::abs(player_->GetPosition().x - enemyPos.x) + std::abs(player_->GetPosition().y - enemyPos.y);
            float d2 = std::abs(player2_->GetPosition().x - enemyPos.x) + std::abs(player2_->GetPosition().y - enemyPos.y);
            return (d1 < d2) ? player_.get() : player2_.get();
        }
        if (p1Alive) return player_.get();
        if (p2Alive) return player2_.get();
        return nullptr; // Both dead or absent
    };

    // Update Goombas (including dying ones — they run their own timer)
    for (auto& g : goombas_) {
        if (g->IsActive()) {
            Player* target = GetTargetPlayer(g->GetPosition());
            g->SetPlayerBody(target ? &target->GetPhysicsBody() : nullptr);
            g->Update(deltaTime);
        }
    }
    
    // Update Koopas
    for (auto& k : koopas_) {
        if (k->IsActive()) {
            Player* target = GetTargetPlayer(k->GetPosition());
            k->SetPlayerBody(target ? &target->GetPhysicsBody() : nullptr);
            k->Update(deltaTime);
        }
    }

    // Update Buzzy Beetle
    for (auto& b : buzzyBeetles_) {
        if (b->IsActive()) {
            Player* target = GetTargetPlayer(b->GetPosition());
            b->SetPlayerBody(target ? &target->GetPhysicsBody() : nullptr);
            b->Update(deltaTime);
        }
    }

    // Update Piranha Plants
    for (auto& p : piranhas_) {
        if (p->IsActive()) {
            Player* target = GetTargetPlayer(p->GetPosition());
            p->SetPlayerBody(target ? &target->GetPhysicsBody() : nullptr);
            p->Update(deltaTime);
        }
    }

    // Gather active players for boss and entity updates
    std::vector<Player*> currentActivePlayers;
    if (player_ && !player_->IsDead()) currentActivePlayers.push_back(player_.get());
    if (isMultiplayer_ && player2_ && !player2_->IsDead()) currentActivePlayers.push_back(player2_.get());

    if (dragonBoss_ && dragonBoss_->IsActive()) {
        dragonBoss_->SetPlayerRef(GetTargetPlayer(dragonBoss_->GetPosition()));
        dragonBoss_->UpdateAI(currentActivePlayers, deltaTime);
        dragonBoss_->Update(deltaTime);

        if (dragonBoss_->ConsumeItemScatterRequest()) {
            Vector2 origin = dragonBoss_->GetPosition();

            // A few coins...
            for (int i = 0; i < 4; ++i) {
                auto c_coin = std::make_unique<Coin>();
                c_coin->SetPosition({ origin.x + (i - 1.5f) * 24.0f, origin.y - 20.0f });
                c_coin->SetPopping(true, -280.0f - i * 20.0f); // slight stagger for a scatter feel
                coins_.push_back(std::move(c_coin));
            }
            // ...plus one power-up, reusing the existing Mushroom spawn pattern.
            auto m = std::make_unique<Mushroom>();
            m->SetPosition({ origin.x, origin.y - 40.0f });
            m->SetCollisionGrid(&tileMap.GetBlockGrid());
            m->SetActive(true);
            mushrooms_.push_back(std::move(m));
            AudioManager::PlaySFX(AudioKey::POWERUP_APPEARS);
        }

        if (dragonBoss_->ConsumeCoinBurstRequest()) {
            Vector2 origin = dragonBoss_->GetPosition();
            int count = dragonBoss_->GetDeathCoinCount();
            for (int i = 0; i < count; ++i) {
                auto c_coin = std::make_unique<Coin>();
                float angleOffset = (i - count / 2.0f) * 15.0f; // spread horizontally
                c_coin->SetPosition({ origin.x + angleOffset, origin.y - 20.0f });
                c_coin->SetPopping(true, -300.0f - (float)(i % 5) * 30.0f);
                coins_.push_back(std::move(c_coin));
            }
        }

        Vector2 flameMouth;
        float flameDir = -1.0f;
        float flameGrowth = 0.0f;
        bool flameEnded = false;
        if (dragonBoss_->GetFlameStreamUpdate(flameMouth, flameDir, flameGrowth, flameEnded)) {
            if (flameEnded) {
                dragonFlames_.clear();
            } else {
                if (dragonFlames_.empty()) {
                    dragonFlames_.push_back(std::make_unique<DragonFlame>(flameMouth, flameDir));
                }
                if (!dragonFlames_.empty()) {
                    dragonFlames_[0]->SetMouthAnchor(flameMouth, flameDir, flameGrowth);
                }
            }
        }

        Vector2 flameOrigin;
        float flameDirLegacy = -1.0f;
        if (dragonBoss_->ConsumeFlameRequest(flameOrigin, flameDirLegacy)) {
            dragonFlames_.push_back(std::make_unique<DragonFlame>(flameOrigin, flameDirLegacy));
        }

        Vector2 swOrigin;
        float swFloorY = 0.0f;
        if (dragonBoss_->ConsumeShockwaveRequest(swOrigin, swFloorY)) {
            shockwaveManager_.Trigger(swOrigin, swFloorY, 800.0f, 440.0f);
        }

        Vector2 fbOrigin;
        if (dragonBoss_->ConsumeFireballRequest(fbOrigin)) {
            fireballs_.push_back(std::make_unique<Fireball>(fbOrigin, -1.0f));
        }
    }

    // Update shockwave ripple across blocks and players
    shockwaveManager_.Update(deltaTime, tileMap.GetBlockGrid(), currentActivePlayers);

    // Update dragon flames
    for (auto& flame : dragonFlames_) {
        if (!flame->IsActive()) continue;
        flame->Update(deltaTime);
        for (Player* p : currentActivePlayers) {
            if (p && !p->IsDead() && flame->CanHurtPlayer(*p) && flame->Overlaps(*p)) {
                p->TakeDamage();
            }
        }
    }
    dragonFlames_.erase(std::remove_if(dragonFlames_.begin(), dragonFlames_.end(),
        [](const std::unique_ptr<DragonFlame>& f) { return !f->IsActive(); }), dragonFlames_.end());

    

    // Update block grid
    tileMap.GetBlockGrid().Update(deltaTime);

    // Update debris particles
    for (auto& d : debrisList_) {
        d.Update(deltaTime);
    }
    debrisList_.erase(std::remove_if(debrisList_.begin(), debrisList_.end(),
        [](const DebrisPiece& d) { return !d.active; }), debrisList_.end());

    // Update score popups
    for (auto& popup : scorePopups_) {
        popup.timer += deltaTime;
    }
    scorePopups_.erase(std::remove_if(scorePopups_.begin(), scorePopups_.end(),
        [](const ScorePopup& p) { return p.timer >= 1.0f; }), scorePopups_.end());

    auto HandleBlockHit = [&](Player* p) {
        if (!p || p->IsDead() || !p->CanHitBlock() || !p->GetPhysicsBody().hitCeiling) return;
        
        Rectangle hitRect = p->GetPhysicsBody().hitCeilingRect;
        float headX = p->GetPosition().x + p->GetSize().x / 2.0f;
        
        int col = (int)(headX / tileMap.GetTileSize());
        int row = (int)(hitRect.y / tileMap.GetTileSize());
        Block* block = tileMap.GetBlockGrid().GetBlock(col, row);
        
        if (block && headX >= hitRect.x && headX <= hitRect.x + hitRect.width) {
            bool isSmall = p->IsSmall();
            
            if (block->IsLucky()) {
                if (block->Bump()) {
                    auto* lucky = dynamic_cast<Luckyblock*>(block);
                    LuckyContents contents = lucky ? lucky->GetLastContents() : LuckyContents::Coin;

                    float blockX = (float)(col * tileMap.GetTileSize());
                    float blockTopY = (float)(row * tileMap.GetTileSize());

                    if (contents == LuckyContents::Mushroom) {
                        if (!isSmall) {
                            // Upgrade to FireFlower if player is not small
                            auto f = std::make_unique<FireFlower>();
                            f->SetPosition({ blockX, blockTopY });
                            f->StartEmerging(blockTopY);
                            fireFlowers_.push_back(std::move(f));
                            AudioManager::PlaySFX(AudioKey::POWERUP_APPEARS);
                        } else {
                            auto m = std::make_unique<Mushroom>();
                            m->SetPosition({ blockX, blockTopY });
                            m->SetCollisionGrid(&tileMap.GetBlockGrid());
                            m->StartEmerging(blockTopY);
                            mushrooms_.push_back(std::move(m));
                            AudioManager::PlaySFX(AudioKey::POWERUP_APPEARS);
                        }
                    } else if (contents == LuckyContents::Starman) {
                        auto s = std::make_unique<Starman>();
                        s->SetPosition({ blockX, blockTopY });
                        s->SetCollisionGrid(&tileMap.GetBlockGrid());
                        s->StartEmerging(blockTopY);
                        starmen_.push_back(std::move(s));
                        AudioManager::PlaySFX(AudioKey::POWERUP_APPEARS);
                    } else {
                        auto c_coin = std::make_unique<Coin>();
                        c_coin->SetPosition({ (float)(col * tileMap.GetTileSize()), hitRect.y - 16.0f * Global::GAME_SCALE });
                        c_coin->SetSize({ 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE });
                        c_coin->SetPopping(true, -350.0f);
                        c_coin->SetAwardsScoreOnCollect(false);
                        coins_.push_back(std::move(c_coin));
                        score += 100;
                    }
                }
                p->SetCanHitBlock(false);
            } else {
                if (isSmall) {
                    block->Bump();
                } else {
                    std::string texKey;
                    Rectangle srcRect = { 0, 0, 16, 16 };
                    bool foundTile = tileMap.RemoveTileAt(col, row, texKey, srcRect);
                    
                    Rectangle blockWorldRect = {
                        (float)(col * tileMap.GetTileSize()),
                        (float)(row * tileMap.GetTileSize()),
                        (float)tileMap.GetTileSize(),
                        (float)tileMap.GetTileSize()
                    };
                    
                    if (foundTile) {
                        SpawnBlockDebris(debrisList_, blockWorldRect, texKey, srcRect);
                    } else {
                        SpawnBlockDebris(debrisList_, blockWorldRect, "luckyblock", { 64.0f, 0.0f, 16.0f, 16.0f });
                    }

                    tileMap.GetBlockGrid().SetBlock(col, row, nullptr);
                    score += 50;
                }
                p->SetCanHitBlock(false);
            }
        }
    };

    HandleBlockHit(player_.get());
    if (isMultiplayer_ && player2_) {
        HandleBlockHit(player2_.get());
    }
    
    // Gather active players for shooting
    std::vector<Player*> activePlayersForShoot;
    if (player_ && !player_->IsDead()) activePlayersForShoot.push_back(player_.get());
    if (isMultiplayer_ && player2_ && !player2_->IsDead()) activePlayersForShoot.push_back(player2_.get());

    for (Player* p : activePlayersForShoot) {
        if (p->WantsToShoot()) {
            p->ConsumeShootRequest();
            if (p->CanShootFireball()) {
                // Limit to 2 player fireballs on screen total
                if (playerFireballs_.size() < 2) {
                    float dir = (p->GetFacing() == FacingDirection::Right) ? 1.0f : -1.0f;
                    Vector2 spawnPos = { p->GetPosition().x + (p->GetSize().x / 2.0f) + (dir * 8.0f), p->GetPosition().y + p->GetSize().y / 2.0f };
                    auto fb = std::make_unique<PlayerFireball>(spawnPos, dir);
                    fb->SetActive(true);
                    playerFireballs_.push_back(std::move(fb));
                    AudioManager::PlaySFX(AudioKey::FIREBALL);
                    p->PlayShootAnimation();
                }
            }
        }
    }

    // Cleanup dead player fireballs before updating to maintain the 2-fireball limit correctly
    playerFireballs_.erase(std::remove_if(playerFireballs_.begin(), playerFireballs_.end(),
        [](const std::unique_ptr<PlayerFireball>& fb) { return !fb->IsActive(); }), playerFireballs_.end());

    for (auto& m : mushrooms_) {
        if (m->IsActive()) {
            m->Update(deltaTime);
        }
    }
    for (auto& s : starmen_) {
        if (s->IsActive()) {
            s->Update(deltaTime);
        }
    }
    for (auto& f : fireFlowers_) {
        if (f->IsActive()) {
            f->Update(deltaTime);
        }
    }
    for (auto& fb : playerFireballs_) {
        if (fb->IsActive()) {
            fb->Update(deltaTime);
            // Handle fireball bouncing
            Rectangle fbRect = fb->GetRect();
            int bcol = (int)(fbRect.x / tileMap.GetTileSize());
            int brow = (int)((fbRect.y + fbRect.height) / tileMap.GetTileSize());
            Block* floorBlock = tileMap.GetBlockGrid().GetBlock(bcol, brow);
            if (floorBlock || fbRect.y + fbRect.height >= tileMap.GetPixelHeight()) {
                // Bounce
                Vector2 vel = fb->GetVelocity();
                vel.y = -350.0f; // Bounce strength
                fb->SetVelocity(vel);
            }
            
            // Handle fireball hitting walls (destroy)
            int sideCol = (fb->GetVelocity().x > 0) ? (int)((fbRect.x + fbRect.width) / tileMap.GetTileSize()) : (int)(fbRect.x / tileMap.GetTileSize());
            int sideRow = (int)((fbRect.y + fbRect.height/2.0f) / tileMap.GetTileSize());
            Block* wallBlock = tileMap.GetBlockGrid().GetBlock(sideCol, sideRow);
            if (wallBlock) {
                fb->Explode();
            }
            
            // Advance position via velocity manually since Projectile isn't updated by PhysicsEngine
            Vector2 pos = { fbRect.x + fb->GetVelocity().x * deltaTime, fbRect.y + fb->GetVelocity().y * deltaTime };
            fb->SetPosition(pos);
        }
    }

    // Gather active players for interactions
    std::vector<Player*> activePlayers;
    if (player_ && !player_->IsDead()) activePlayers.push_back(player_.get());
    if (isMultiplayer_ && player2_ && !player2_->IsDead()) activePlayers.push_back(player2_.get());

    // Update Boss Battle State Machine
    bossBattleCtrl_.Update(deltaTime, activePlayers, dragonBoss_.get());

    // Entity interaction: players vs goombas/koopas/mushroom/boss
    for (Player* p : activePlayers) {
        for (auto& g : goombas_) {
            if (g->IsActive() && !g->IsDying() && !g->IsUpsideDownDead() && p->Overlaps(*g)) {
                bool wasDying = g->IsDying();
                bool wasUpsideDown = g->IsUpsideDownDead();
                p->InteractWith(*g);
                if ((!wasDying && g->IsDying()) || (!wasUpsideDown && g->IsUpsideDownDead())) {
                    score += 100;
                    scorePopups_.push_back({g->GetPosition(), 0.0f, 100});
                }
            }
        }
        for (auto& k : koopas_) {
            if (k->IsActive() && !k->IsUpsideDownDead() && p->Overlaps(*k)) {
                KoopaShellState prevState = k->GetState();
                bool wasUpsideDown = k->IsUpsideDownDead();
                p->InteractWith(*k);
                if (prevState != KoopaShellState::Hiding && k->GetState() == KoopaShellState::Hiding) {
                    score += 100;
                }
            }
        }
        for (auto& b : buzzyBeetles_) {
            if (b->IsActive() && p->Overlaps(*b)) {
                p->InteractWith(*b);
            }
        }
        for (auto& pir : piranhas_) {
            if (pir->IsActive() && p->Overlaps(*pir)) {
                bool wasActive = pir->IsActive();
                p->InteractWith(*pir);
                if (wasActive && !pir->IsActive()) {
                    score += 200;
                    scorePopups_.push_back({pir->GetPosition(), 0.0f, 200});
                }
            }
        }
        for (auto& b : bullets_) {
            if (b->IsActive() && p->Overlaps(*b)) {
                p->InteractWith(*b);
            }
        }
        for (auto& m : mushrooms_) {
            if (m->IsActive() && p->Overlaps(*m)) {
                bool wasActive = m->IsActive();
                p->InteractWith(*m);
                if (wasActive && !m->IsActive()) {
                    score += 1000;
                    scorePopups_.push_back({m->GetPosition(), 0.0f, 1000});
                }
            }
        }
        for (auto& s : starmen_) {
            if (s->IsActive() && p->Overlaps(*s)) {
                bool wasActive = s->IsActive();
                p->InteractWith(*s);
                if (wasActive && !s->IsActive()) {
                    score += 1000;
                    scorePopups_.push_back({s->GetPosition(), 0.0f, 1000});
                }
            }
        }
        for (auto& f : fireFlowers_) {
            if (f->IsActive() && p->Overlaps(*f)) {
                bool wasActive = f->IsActive();
                p->InteractWith(*f);
                if (wasActive && !f->IsActive()) {
                    score += 1000;
                    scorePopups_.push_back({f->GetPosition(), 0.0f, 1000});
                }
            }
        }
        if (dragonBoss_ && dragonBoss_->IsActive() && !dragonBoss_->IsDead() && p->Overlaps(*dragonBoss_)) {
            bool wasDead = dragonBoss_->IsDead();
            p->InteractWith(*dragonBoss_);
            if (!wasDead && dragonBoss_->IsDead()) {
                score += 1000;
                scorePopups_.push_back({dragonBoss_->GetPosition(), 0.0f, 1000});
            }
        }
    }

    // Unified Enemy-to-Enemy physical collision resolution
    std::vector<GroundEnemy*> activeEnemies;
    for (auto& g : goombas_) if (g->IsActive() && !g->IsDying()) activeEnemies.push_back(g.get());
    for (auto& k : koopas_) if (k->IsActive()) activeEnemies.push_back(k.get());
    for (auto& b : buzzyBeetles_) if (b->IsActive()) activeEnemies.push_back(b.get());

    // Fireball vs Enemy collision
    for (auto& fb : playerFireballs_) {
        if (fb->IsActive() && !fb->IsExploded()) {
            for (auto* e : activeEnemies) {
                if (CheckCollisionRecs(fb->GetRect(), e->GetRect())) {
                    if (auto* k = dynamic_cast<KoopaShell*>(e)) {
                        fb->OnHitShell(*k);
                        score += 100;
                        scorePopups_.push_back({k->GetPosition(), 0.0f, 100});
                        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
                    } else if (auto* b = dynamic_cast<BuzzyBeetle*>(e)) {
                        fb->OnHitEnemy(*b);
                        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
                    } else {
                        fb->OnHitEnemy(*e);
                        score += 100;
                        scorePopups_.push_back({e->GetPosition(), 0.0f, 100});
                        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
                    }
                    break;
                }
            }
            if (!fb->IsExploded()) {
                for (auto& pir : piranhas_) {
                    if (pir->IsActive() && pir->IsExposedOrMoving() && CheckCollisionRecs(fb->GetRect(), pir->GetRect())) {
                        fb->Explode();
                        pir->SetActive(false); // Disappear instantly
                        score += 200;
                        scorePopups_.push_back({pir->GetPosition(), 0.0f, 200});
                        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
                        break;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < activeEnemies.size(); ++i) {
        for (size_t j = i + 1; j < activeEnemies.size(); ++j) {
            auto* e1 = activeEnemies[i];
            auto* e2 = activeEnemies[j];
            
            // Do not push apart if one is a sliding shell (let the interact logic handle it)
            bool e1Sliding = (dynamic_cast<KoopaShell*>(e1) && static_cast<KoopaShell*>(e1)->GetState() == KoopaShellState::Sliding);
            bool e2Sliding = (dynamic_cast<KoopaShell*>(e2) && static_cast<KoopaShell*>(e2)->GetState() == KoopaShellState::Sliding);
            if (e1Sliding || e2Sliding) continue;

            if (e1->Overlaps(*e2)) {
                // Calculate horizontal penetration
                float e1Center = e1->GetPosition().x + e1->GetSize().x / 2.0f;
                float e2Center = e2->GetPosition().x + e2->GetSize().x / 2.0f;
                float dist = std::abs(e1Center - e2Center);
                float minDist = (e1->GetSize().x + e2->GetSize().x) / 2.0f;
                
                // Only push if there's significant overlap horizontally, and vertically they are aligned
                float e1CenterY = e1->GetPosition().y + e1->GetSize().y / 2.0f;
                float e2CenterY = e2->GetPosition().y + e2->GetSize().y / 2.0f;
                if (std::abs(e1CenterY - e2CenterY) < 16.0f) {
                    if (dist < minDist) {
                        float push = (minDist - dist) / 2.0f;
                        if (e1Center < e2Center) {
                            e1->SetPosition({ e1->GetPosition().x - push, e1->GetPosition().y });
                            e2->SetPosition({ e2->GetPosition().x + push, e2->GetPosition().y });
                        } else {
                            e1->SetPosition({ e1->GetPosition().x + push, e1->GetPosition().y });
                            e2->SetPosition({ e2->GetPosition().x - push, e2->GetPosition().y });
                        }
                        e1->SyncPhysicsBody();
                        e2->SyncPhysicsBody();
                        
                        // Bounce off if idle (not tracking)
                        if (!e1->GetPhysicsBody().isTracking) {
                            e1->GetPhysicsBody().aiDirection = (e1Center < e2Center) ? -1 : 1;
                        }
                        if (!e2->GetPhysicsBody().isTracking) {
                            e2->GetPhysicsBody().aiDirection = (e2Center < e1Center) ? -1 : 1;
                        }
                    }
                }
            }
        }
    }

    // Entity interaction: Koopa vs Goomba
    for (auto& k : koopas_) {
        if (k->IsActive() && k->GetState() == KoopaShellState::Sliding) {
            for (auto& g : goombas_) {
                if (g->IsActive() && !g->IsDying() && !g->IsUpsideDownDead() && k->Overlaps(*g)) {
                    bool wasDying = g->IsDying();
                    bool wasUpsideDown = g->IsUpsideDownDead();
                    k->InteractWith(*g);
                    if ((!wasDying && g->IsDying()) || (!wasUpsideDown && g->IsUpsideDownDead())) {
                        score += 100;
                        scorePopups_.push_back({g->GetPosition(), 0.0f, 100});
                    }
                }
            }
        }
    }

    // Entity interaction: Koopa vs Koopa
    for (size_t i = 0; i < koopas_.size(); ++i) {
        auto& kA = koopas_[i];
        if (!kA->IsActive() || kA->GetState() != KoopaShellState::Sliding) continue;
        for (size_t j = i + 1; j < koopas_.size(); ++j) {
            auto& kB = koopas_[j];
            if (!kB->IsActive()) continue;
            if (kA->Overlaps(*kB)) {
                kA->InteractWith(*kB);
            }
        }
    }

    for (auto& k : koopas_) {
        if (k->IsActive() && k->GetState() == KoopaShellState::Sliding) {
            for (auto& b : buzzyBeetles_) {
                if (b->IsActive() && k->Overlaps(*b)) {
                    k->InteractWith(*b); // BuzzyBeetle is invincible — shell bounces off
                }
            }
            for (auto& pir : piranhas_) {
                if (pir->IsActive() && pir->IsExposedOrMoving() && k->Overlaps(*pir)) {
                    k->InteractWith(*pir);
                }
            }
            for (auto& b : bullets_) {
                if (b->IsActive() && k->Overlaps(*b)) {
                    k->InteractWith(*b);
                }
            }
        }
        if (k->IsActive() && k->GetState() == KoopaShellState::Sliding &&
            dragonBoss_ && dragonBoss_->IsActive() && k->Overlaps(*dragonBoss_)) {
            bool wasDead = dragonBoss_->IsDead();
            k->InteractWith(*dragonBoss_);
            if (!wasDead && dragonBoss_->IsDead()) {
                score += 1000;
                scorePopups_.push_back({dragonBoss_->GetPosition(), 0.0f, 1000});
            }
        }
    }

    // Update and collect Coins
    for (auto& coin : coins_) {
        if (coin->IsActive()) {
            coin->Update(deltaTime);
            Rectangle cRect = { coin->GetPosition().x, coin->GetPosition().y, coin->GetSize().x, coin->GetSize().y };
            for (Player* p : activePlayers) {
                if (CheckCollisionRecs(p->GetPhysicsBody().GetRect(), cRect)) {
                    coin->SetActive(false);
                    AudioManager::PlaySFX(AudioKey::HIT_COIN);
                    if (coin->AwardsScoreOnCollect()) {
                        score += 200; // Original mario coin gives 200 score
                        scorePopups_.push_back({coin->GetPosition(), 0.0f, 200});
                    }
                    break;
                }
            }
        }
    }

    for (auto& fb : fireballs_) {
        if (!fb->IsActive()) continue;
        fb->Update(deltaTime);
        if (fb->IsExploded()) continue; // already resolving its explosion visual, skip collision checks

        Rectangle fbRect = fb->GetRect();
        bool justExploded = false;

        // 1. Map geometry
        if (RectOverlapsSolidBlock(fbRect, tileMap.GetBlockGrid())) {
            fb->OnHitSolid();
            justExploded = true;
        }

        // 2. Koopa shells
        if (!justExploded) {
            for (auto& k : koopas_) {
                if (k->IsActive() && CheckCollisionRecs(fbRect, k->GetRect())) {
                    fb->OnHitShell(*k);
                    justExploded = true;
                    break;
                }
            }
        }

        // 3. Ground enemies (Goomba/BuzzyBeetle — both derive from GroundEnemy)
        if (!justExploded) {
            for (auto& g : goombas_) {
                if (g->IsActive() && !g->IsDying() && CheckCollisionRecs(fbRect, g->GetRect())) {
                    fb->OnHitEnemy(*g);
                    justExploded = true;
                    break;
                }
            }
        }
        if (!justExploded) {
            for (auto& b : buzzyBeetles_) {
                if (b->IsActive() && CheckCollisionRecs(fbRect, b->GetRect())) {
                    fb->OnHitEnemy(*b);
                    justExploded = true;
                    break;
                }
            }
        }

        // 4. Direct player contact
        bool hitPlayerDirectly = false;
        if (!justExploded) {
            for (Player* p : activePlayers) {
                if (CheckCollisionRecs(fbRect, p->GetRect())) {
                    hitPlayerDirectly = true;
                    if (fb->CanHurtPlayer(*p)) {
                        p->TakeDamage();
                    }
                    fb->Explode();
                    justExploded = true;
                    break;
                }
            }
        }

        // 5. AOE — only if something ELSE triggered the explosion (avoid double-hit same frame)
        if (justExploded && !hitPlayerDirectly && fb->GetAOERadius() > 0.0f) {
            for (Player* p : activePlayers) {
                float dx = p->GetPosition().x - fb->GetPosition().x;
                float dy = p->GetPosition().y - fb->GetPosition().y;
                if (std::sqrt(dx * dx + dy * dy) <= fb->GetAOERadius() && fb->CanHurtPlayer(*p)) {
                    p->TakeDamage();
                }
            }
        }
    }

    // Trigger and spawn Bullets when player's block column equals trigger's block column
    float tileSize = (float)tileMap.GetTileSize();
    if (tileSize <= 0.0f) tileSize = Global::TILE_SIZE * Global::GAME_SCALE;

    for (auto& trigger : bulletTriggers_) {
        if (!trigger.triggered) {
            int triggerBlockX = (int)(trigger.triggerX / tileSize);
            for (Player* p : activePlayers) {
                int playerBlockX = (int)((p->GetPosition().x + p->GetSize().x / 2.0f) / tileSize);
                if (playerBlockX == triggerBlockX) {
                    trigger.triggered = true;

                    const Camera2D& cam = view.GetRawCamera();
                    float cameraZoom = (cam.zoom > 0.0f) ? cam.zoom : 1.0f;
                    float worldWidth = (float)GetScreenWidth() / cameraZoom;
                    float bulletWidth = 16.0f * Global::GAME_SCALE;

                    float spawnX = 0.0f;
                    if (trigger.direction > 0.0f) {
                        // Shoot from left outside of camera view towards right
                        spawnX = view.GetWorldLeft() - bulletWidth - 10.0f;
                    } else {
                        // Shoot from right outside of camera view towards left
                        spawnX = view.GetWorldLeft() + worldWidth + 10.0f;
                    }

                    auto bullet = std::make_unique<Bullet>(Vector2{ spawnX, trigger.spawnY }, trigger.direction);
                    bullets_.push_back(std::move(bullet));
                    break;
                }
            }
        }
    }

    // Resolve Bullet block collisions
    for (auto& bullet : bullets_) {
        if (!bullet->IsActive()) continue;

        Rectangle bRect = bullet->GetRect();

        // Map geometry (Solid blocks) -> Disappear on hit
        // Only check block collision if the bullet is within map horizontal bounds
        if (bullet->GetPosition().x >= 0.0f && bullet->GetPosition().x + bullet->GetSize().x <= tileMap.GetPixelWidth()) {
            if (RectOverlapsSolidBlock(bRect, tileMap.GetBlockGrid())) {
                bullet->OnHitSolid();
                continue;
            }
        }
    }

    // Cleanup inactive bullets
    bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
        [](const std::unique_ptr<Bullet>& b) { return !b->IsActive(); }), bullets_.end());

    // Cleanup dead fireballs
    fireballs_.erase(std::remove_if(fireballs_.begin(), fireballs_.end(),[](const std::unique_ptr<Fireball>& fb) { return !fb->IsActive(); }), fireballs_.end());

    // Check collisions with Fire
    for (auto& fire : fires_) {
        for (Player* p : activePlayers) {
            if (CheckCollisionRecs(p->GetPhysicsBody().GetRect(), fire->GetRect())) {
                p->SetDead(true);
                AudioManager::PlaySFX(AudioKey::MARIO_DIE);
            }
        }
    }

    // Kill players instantly when they fall into a pit (map bottom border)
    for (Player* p : activePlayers) {
        if (p->GetPosition().y >= tileMap.GetBorderBottom()) {
            p->TakeDamage();
            if (!p->IsDead()) {
                p->SetDead(true);
                AudioManager::PlaySFX(AudioKey::MARIO_DIE);
            }
        }
    }

    // Game Over: in multiplayer, wait until BOTH players have fallen off-screen
    {
        bool p1OffScreen = player_->IsDead() && player_->GetPosition().y > tileMap.GetBorderBottom() + 100;
        bool p2OffScreen = !isMultiplayer_ || !player2_ ||
                           (player2_->IsDead() && player2_->GetPosition().y > tileMap.GetBorderBottom() + 100);
        if (p1OffScreen && p2OffScreen) {
            isGameOver = true;
            AudioManager::StopBGM();
        }
    }

    if (princess_) princess_->Update(deltaTime); // tick idle animation

    if (!isGameWon && !isGameOver && princess_) {
        for (Player* p : activePlayers) {
            float dx = (p->GetPosition().x + p->GetSize().x / 2.0f) -
                       (princess_->GetPosition().x + princess_->GetSize().x / 2.0f);
            float dy = (p->GetPosition().y + p->GetSize().y / 2.0f) -
                       (princess_->GetPosition().y + princess_->GetSize().y / 2.0f);
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= princess_->GetInteractionRadius()) {
                isGameWon = true;
                AudioManager::StopBGM();
                Global::gameStateManager->PushState(std::make_unique<LevelCompleteState>(
                    this, currentLevel.GetLevelNumber(), characterId_, score, timeLeft));
                break;
            }
        }
    }

    // GoalPipe Win Condition
    if (goalPipe_ && !isGameWon && !isGameOver) {
        if (!goalPipe_->IsTriggered()) {
            for (Player* p : activePlayers) {
                if (CheckCollisionRecs(p->GetPhysicsBody().GetRect(), goalPipe_->GetRect())) {
                    winningPlayer_ = p;
                    goalPipe_->Trigger(winningPlayer_->GetPosition());
                    AudioManager::PlaySFX(AudioKey::PIPE_TRAVEL);
                    break;
                }
            }
        } else if (winningPlayer_) {
            // During slide animation
            goalPipe_->Update(deltaTime);
            winningPlayer_->SetPosition(goalPipe_->GetPlayerAnimPos());
            
            // Stop other players from moving while win animation plays
            for (Player* p : activePlayers) {
                if (p != winningPlayer_) {
                    p->GetPhysicsBody().velocity = {0,0};
                    p->SetAnimation(p->GetPoseAnimation());
                }
            }

            if (goalPipe_->IsAnimationComplete()) {
                isGameWon = true;
                AudioManager::StopBGM();
                Global::gameStateManager->PushState(std::make_unique<LevelCompleteState>(
                    this, currentLevel.GetLevelNumber(), characterId_, score, timeLeft));
            }
        }
    }

    // Flagpole Win Condition
    if (flagpole_ && !isGameWon && !isGameOver) {
        if (!flagpole_->IsSliding() && !flagpole_->IsComplete()) {
            for (Player* p : activePlayers) {
                if (CheckCollisionRecs(p->GetPhysicsBody().GetRect(), flagpole_->GetTriggerBounds())) {
                    winningPlayer_ = p;
                    flagpole_->Trigger(winningPlayer_->GetPosition().y);
                    AudioManager::PlaySFX(AudioKey::DOWN_FLAG_POLE);
                    winningPlayer_->GetPhysicsBody().velocity = {0,0};
                    break;
                }
            }
        } else if (flagpole_->IsSliding() && winningPlayer_) {
            flagpole_->Update(deltaTime);
            // Lock winning player to the pole x position and flag y position
            winningPlayer_->SetPosition({ flagpole_->GetPoleX() - winningPlayer_->GetSize().x / 2.0f, flagpole_->GetFlagY() });
            winningPlayer_->GetPhysicsBody().velocity = {0,0};
            winningPlayer_->SetAnimation(winningPlayer_->GetSlideAnimation());

            // Stop other players from moving while win animation plays
            for (Player* p : activePlayers) {
                if (p != winningPlayer_) {
                    p->GetPhysicsBody().velocity = {0,0};
                    p->SetAnimation(p->GetPoseAnimation());
                }
            }

            if (flagpole_->IsComplete()) {
                isGameWon = true;
                AudioManager::StopBGM();
                Global::gameStateManager->PushState(std::make_unique<LevelCompleteState>(
                    this, currentLevel.GetLevelNumber(), characterId_, score, timeLeft));
            }
        }
    }

    // Dynamic Camera: If in Intro cutscene, focus & zoom on boss then pan back to player; otherwise track fight/player
    float camTargetX = player_->GetPosition().x;
    float camTargetY = player_->GetPosition().y;
    float camZoomMult = 1.0f;

    bool isIntroCutscene = (bossBattleCtrl_.GetPhase() == BossBattlePhase::Intro) &&
                           dragonBoss_ && dragonBoss_->IsActive();

    if (isIntroCutscene) {
        float elapsed = BossBattleController::kIntroDuration - bossBattleCtrl_.GetPhaseTimer();
        Vector2 bPos = dragonBoss_->GetPosition();
        Vector2 bSize = dragonBoss_->GetSize();
        Vector2 bossCenter = { bPos.x + bSize.x * 0.5f, bPos.y + bSize.y * 0.45f };

        Vector2 pPos = player_->GetPosition();
        Vector2 pSize = player_->GetSize();
        Vector2 playerCenter = { pPos.x + pSize.x * 0.5f, pPos.y + pSize.y * 0.5f };

        if (elapsed < 1.8f) {
            // Stage 1: Zoom in on Boss while it roars
            camTargetX = bossCenter.x;
            camTargetY = bossCenter.y;
            camZoomMult = 1.6f;
        } else {
            // Stage 2: Smoothly glide camera from Boss to Player, and unzoom (1.6x -> 1.0x)
            float t = (elapsed - 1.8f) / 1.8f;
            t = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
            float ease = t * t * (3.0f - 2.0f * t); // Smooth smoothstep
            camTargetX = bossCenter.x + (playerCenter.x - bossCenter.x) * ease;
            camTargetY = bossCenter.y + (playerCenter.y - bossCenter.y) * ease;
            camZoomMult = 1.6f - 0.6f * ease;
        }
    } else {
        bool inBossFight = (bossBattleCtrl_.GetPhase() == BossBattlePhase::Fighting) &&
                            dragonBoss_ && dragonBoss_->IsActive() && !dragonBoss_->IsDead() &&
                            (std::abs(player_->GetPosition().x - dragonBoss_->GetPosition().x) < 900.0f);

        if (inBossFight) {
            float sumX = dragonBoss_->GetPosition().x + dragonBoss_->GetSize().x / 2.0f;
            float sumY = dragonBoss_->GetPosition().y + dragonBoss_->GetSize().y / 2.0f;
            int count = 1;

            if (player_ && !player_->IsDead()) {
                sumX += player_->GetPosition().x + player_->GetSize().x / 2.0f;
                sumY += player_->GetPosition().y + player_->GetSize().y / 2.0f;
                count++;
            }
            if (isMultiplayer_ && player2_ && !player2_->IsDead()) {
                sumX += player2_->GetPosition().x + player2_->GetSize().x / 2.0f;
                sumY += player2_->GetPosition().y + player2_->GetSize().y / 2.0f;
                count++;
            }

            camTargetX = sumX / (float)count;
            camTargetY = sumY / (float)count;
        } else {
            bool p1Alive = player_ && !player_->IsDead();
            bool p2Alive = isMultiplayer_ && player2_ && !player2_->IsDead();

            if (p1Alive && p2Alive) {
                camTargetX = (player_->GetPosition().x + player2_->GetPosition().x) / 2.0f;
                camTargetY = (player_->GetPosition().y + player2_->GetPosition().y) / 2.0f;
            } else if (p1Alive) {
                camTargetX = player_->GetPosition().x;
                camTargetY = player_->GetPosition().y;
            } else if (p2Alive) {
                camTargetX = player2_->GetPosition().x;
                camTargetY = player2_->GetPosition().y;
            } else {
                camTargetX = player_->GetPosition().x;
                camTargetY = player_->GetPosition().y;
            }
        }
    }

    if (firstCameraInit_) {
        smoothedCamX_ = camTargetX;
        smoothedCamY_ = camTargetY;
        firstCameraInit_ = false;
    } else {
        // Smoothly interpolate towards the target
        float camLerpSpeed = isIntroCutscene ? 10.0f : 6.0f; 
        smoothedCamX_ += (camTargetX - smoothedCamX_) * camLerpSpeed * deltaTime;
        smoothedCamY_ += (camTargetY - smoothedCamY_) * camLerpSpeed * deltaTime;
    }
    
    view.Update(smoothedCamX_, smoothedCamY_, camZoomMult);
}

void GameplayState::Draw() {
    ClearBackground(tileMap.GetBackgroundColor());

    view.BeginDraw();

    tileMap.Draw(view.GetWorldLeft(), view.GetWorldTop(), view.GetRawCamera().zoom);

    // Draw interactive blocks
    tileMap.GetBlockGrid().Draw();

    bool clipPlayer = goalPipe_ && goalPipe_->IsTriggered();
    if (clipPlayer) {
        Vector2 pipeScreenPos = GetWorldToScreen2D(goalPipe_->GetPosition(), view.GetRawCamera());
        BeginScissorMode(0, 0, (int)pipeScreenPos.x, GetScreenHeight());
    }

    player_->Draw();
    if (isMultiplayer_ && player2_) {
        player2_->Draw();
    }

    if (clipPlayer) {
        EndScissorMode();
    }

    for (auto& g : goombas_) {
        // Draw alive AND dying goombas (dying ones show squish + "+100" popup)
        if (g->IsActive()) {
            g->Draw();
        }
    }
    for (auto& k : koopas_) {
        if (k->IsActive()) {
            k->Draw();
        }
    }
    for (auto& b : buzzyBeetles_) {
        if (b->IsActive()) {
            b->Draw();
        }
    }
    for (auto& p : piranhas_) {
        if (p->IsActive()) {
            p->Draw();
        }
    }
    if (dragonBoss_ && dragonBoss_->IsActive()) {
        dragonBoss_->Draw();
    }
    shockwaveManager_.Draw();
    for (auto& flame : dragonFlames_) {
        if (flame->IsActive()) flame->Draw();
    }
    for (auto& fb : fireballs_) {
        if (fb->IsActive()) fb->Draw();
    }
    for (auto& bullet : bullets_) {
        if (bullet->IsActive()) bullet->Draw();
    }
    for (auto& bridge : flyingBridges_) {
        bridge->Draw();
    }
    for (auto& coin : coins_) {
        if (coin->IsActive()) {
            coin->Draw();
        }
    }
    for (auto& m : mushrooms_) {
        if (m->IsActive()) {
            m->Draw();
        }
    }
    for (auto& s : starmen_) {
        if (s->IsActive()) {
            s->Draw();
        }
    }
    for (auto& f : fireFlowers_) {
        if (f->IsActive()) {
            f->Draw();
        }
    }
    for (auto& fb : playerFireballs_) {
        if (fb->IsActive()) {
            fb->Draw();
        }
    }

    for (const auto& d : debrisList_) {
        d.Draw();
    }

    for (const auto& popup : scorePopups_) {
        float alpha = 1.0f - (popup.timer / 1.0f);
        if (alpha < 0.0f) alpha = 0.0f;
        float offsetY = -60.0f * popup.timer;
        int fontSize = 24;
        const char* popupText = TextFormat("+%d", popup.score);
        int textW = MeasureText(popupText, fontSize);
        int px = (int)(popup.position.x - textW / 2.0f);
        int py = (int)(popup.position.y + offsetY);
        DrawText(popupText, px, py, fontSize, Fade(YELLOW, alpha));
    }

    // Draw all map Door blocks directly from map entities
    for (const auto& rect : mapDoorBlocks_) {
        bool isExitDoor = (bossBattleCtrl_.HasDoor() && rect.x >= bossBattleCtrl_.GetDoorPos().x - 10.0f);
        bool isOpen = isExitDoor && (bossBattleCtrl_.GetPhase() == BossBattlePhase::RoomCleared);
        if (isOpen) {
            // Open illuminated doorway
            DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, Color{ 25, 15, 35, 200 });
            DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, GOLD);
        } else {
            DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, Color{ 90, 50, 25, 255 });
            DrawRectangle((int)rect.x + 3, (int)rect.y + 3, (int)rect.width - 6, (int)rect.height - 6, Color{ 120, 70, 35, 255 });
            DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, Color{ 40, 20, 10, 255 });
        }
    }

    // Draw Boss Battle Doors (reDoor and Door via BossBattleController)
    bossBattleCtrl_.DrawWorld(dragonBoss_.get());

    if (princess_) princess_->Draw();
    if (goalPipe_) goalPipe_->Draw();
    if (flagpole_) flagpole_->Draw();

    view.EndDraw();

    // Draw HUD
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    int fontSize = (int)(sh * 0.035f);

    DrawText("MARIO", (int)(sw * 0.05f), (int)(sh * 0.03f), fontSize, WHITE);
    DrawText(TextFormat("%06i", score), (int)(sw * 0.05f), (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    const char* worldLabel = "WORLD";
    int worldX = (int)((sw - MeasureText(worldLabel, fontSize)) * 0.5f);
    DrawText(worldLabel, worldX, (int)(sh * 0.03f), fontSize, WHITE);
    
    int levelW = MeasureText(currentLevel.GetDisplayName().c_str(), fontSize);
    DrawText(currentLevel.GetDisplayName().c_str(), (int)(worldX + MeasureText(worldLabel, fontSize)/2 - levelW/2), (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    const char* timeLabel = "TIME";
    int timeX = (int)(sw - MeasureText(timeLabel, fontSize) - sw * 0.05f);
    DrawText(timeLabel, timeX, (int)(sh * 0.03f), fontSize, WHITE);
    DrawText(TextFormat("%03i", (int)timeLeft), timeX + 15, (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    // Boss HP Bar (if in battle)
    bossBattleCtrl_.DrawHUD(dragonBoss_.get(), sw, sh);

    if (isGameOver) {
        DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 150});
        
        int overSize = (int)(sh * 0.15f); // Increased size
        Vector2 goSize = MeasureTextEx(Global::titleFont, "GAME OVER", overSize, 1.0f);
        DrawTextEx(Global::titleFont, "GAME OVER", { (sw - goSize.x) / 2, sh / 2 - goSize.y }, overSize, 1.0f, RED);
        
        const char* prompt = "Press ENTER to return";
        int promptSize = (int)(sh * 0.05f);
        Vector2 pSize = MeasureTextEx(Global::baseFont, prompt, promptSize, 1.0f);
        DrawTextEx(Global::baseFont, prompt, { (sw - pSize.x) / 2, sh / 2 + 50 }, promptSize, 1.0f, WHITE);
    }

}

void GameplayState::Cleanup() {
    AudioManager::StopStarmanBGM();
    goombas_.clear();
    koopas_.clear();
    buzzyBeetles_.clear();
    piranhas_.clear();
    bullets_.clear();
    bulletTriggers_.clear();
    dragonBoss_.reset();
    dragonFlames_.clear();
    shockwaveManager_.Clear();
    fireballs_.clear();
    coins_.clear();
    princess_.reset();
    flyingBridges_.clear();
    goalPipe_.reset();
    mushrooms_.clear();
    fireFlowers_.clear();
    starmen_.clear();
    playerFireballs_.clear();
    fires_.clear();
    player2_.reset();

    bossBattleCtrl_.Reset();
}
