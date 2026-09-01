#include "BossBattleController.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "Game_Objects/Derived_Objects/Enemies/Boss/SpecificBoss/DragonBoss/DragonBoss.h"
#include "AudioManager/AudioManager.h"
#include "Global/Global.h"
#include <algorithm>
#include <cmath>

void BossBattleController::Reset() {
    phase_ = BossBattlePhase::Waiting;
    hasBattle_ = false;
    hasDoor_ = false;
    doorPos_ = { 0.0f, 0.0f };
    hasStartBattle_ = false;
    startBattlePos_ = { 0.0f, 0.0f };
    startBattleX_ = 0.0f;
    phaseTimer_ = 0.0f;
    introRoarStarted_ = false;
}

void BossBattleController::SetDoor(Vector2 pos) {
    doorPos_ = pos;
    hasDoor_ = true;
    hasBattle_ = true;
}

void BossBattleController::SetStartBattle(Vector2 pos) {
    startBattlePos_ = pos;
    startBattleX_ = pos.x;
    hasStartBattle_ = true;
    hasBattle_ = true;
}

void BossBattleController::SetStartBattle(float x) {
    startBattlePos_ = { x, 0.0f };
    startBattleX_ = x;
    hasStartBattle_ = true;
    hasBattle_ = true;
}

void BossBattleController::TeleportPlayers(const std::vector<Player*>& activePlayers) {
    if (!hasStartBattle_ || activePlayers.empty()) return;

    float tileSize = Global::TILE_SIZE * Global::GAME_SCALE; // Tile size in world coordinates

    for (size_t i = 0; i < activePlayers.size(); ++i) {
        Player* p = activePlayers[i];
        if (p && !p->IsDead()) {
            float playerHeight = p->GetSize().y;
            // Align player bottom with tile bottom: Y_bottom = startBattlePos_.y + tileSize
            float alignedY = startBattlePos_.y + tileSize - playerHeight;
            float alignedX = startBattlePos_.x + (float)i * 36.0f;
            p->SetPosition({ alignedX, alignedY });
            p->GetPhysicsBody().velocity = { 0.0f, 0.0f };
            p->GetPhysicsBody().isGrounded = true;
            p->SyncPhysicsBody();
        }
    }
}

void BossBattleController::BeginIntro(const std::vector<Player*>& activePlayers) {
    hasBattle_ = true;
    phase_ = BossBattlePhase::Intro;
    phaseTimer_ = kIntroDuration;
    introRoarStarted_ = false;
    TeleportPlayers(activePlayers);
}

void BossBattleController::StartBattle(const std::vector<Player*>& activePlayers) {
    hasBattle_ = true;
    phase_ = BossBattlePhase::Fighting;
    AudioManager::PlayBGM(AudioKey::BGM_DRAGON_BOSS);
    TeleportPlayers(activePlayers);
}

void BossBattleController::Update(float dt, const std::vector<Player*>& activePlayers, DragonBoss* boss) {
    if (!hasBattle_ && !boss) return;

    // Confine boss strictly within arena boundaries between left and right doors
    if (boss && boss->IsActive()) {
        float doorW = 16.0f * Global::GAME_SCALE;
        float minX = -1.0f;
        float maxX = -1.0f;

        if (hasStartBattle_) {
            minX = startBattleX_;
        }
        if (hasDoor_) {
            maxX = doorPos_.x;
        }

        Vector2 bPos = boss->GetPosition();
        if (minX > 0.0f && bPos.x < minX) {
            bPos.x = minX;
            boss->SetPosition(bPos);
        }
        if (maxX > 0.0f && bPos.x + boss->GetSize().x > maxX) {
            bPos.x = maxX - boss->GetSize().x;
            boss->SetPosition(bPos);
        }
    }

    switch (phase_) {
        case BossBattlePhase::Waiting: {
            bool trigger = false;
            // Immediate trigger if map has startbattle / boss, or when player reaches startBattleX_
            if (hasStartBattle_) {
                for (Player* p : activePlayers) {
                    if (p && !p->IsDead() && p->GetPosition().x >= startBattleX_) {
                        trigger = true;
                        break;
                    }
                }
                // Also auto trigger on map load for quick testing
                trigger = true;
            } else if (boss && boss->IsActive()) {
                trigger = true;
            }

            if (trigger) {
                phase_ = BossBattlePhase::Intro;
                phaseTimer_ = kIntroDuration;
                introRoarStarted_ = false;
                TeleportPlayers(activePlayers);
            }
            break;
        }

        case BossBattlePhase::Intro: {
            phaseTimer_ -= dt;
            if (phaseTimer_ <= 0.0f) {
                phase_ = BossBattlePhase::Fighting;
                AudioManager::PlayBGM(AudioKey::BGM_DRAGON_BOSS);
            }
            break;
        }

        case BossBattlePhase::Fighting: {
            if (boss && boss->IsDead()) {
                phase_ = BossBattlePhase::BossDefeated;
                phaseTimer_ = kDefeatedDuration;
            }
            break;
        }

        case BossBattlePhase::BossDefeated: {
            phaseTimer_ -= dt;
            if (phaseTimer_ <= 0.0f) {
                phase_ = BossBattlePhase::RoomCleared;
            }
            break;
        }

        case BossBattlePhase::RoomCleared: {
            // Battle finished, door remains permanently open
            break;
        }
    }
}

void BossBattleController::AppendDynamicBarriers(std::vector<Rectangle>& platforms, DragonBoss* boss) const {
    // Dynamic collision for doors is provided by mapDoorBlocks_ in GameplayState
}

bool BossBattleController::GetCameraBounds(float& minWorldX, float& maxWorldX) const {
    if (phase_ == BossBattlePhase::Fighting || phase_ == BossBattlePhase::Intro || phase_ == BossBattlePhase::BossDefeated) {
        if (hasStartBattle_ && hasDoor_) {
            minWorldX = startBattleX_;
            maxWorldX = doorPos_.x;
            return true;
        }
    }
    return false;
}

bool BossBattleController::GetRoomCenter(Vector2& outCenter, DragonBoss* boss) const {
    if (phase_ == BossBattlePhase::Waiting && !hasBattle_ && !boss) return false;

    float leftX = 0.0f;
    float rightX = 0.0f;
    bool hasLeft = false;
    bool hasRight = false;

    if (hasStartBattle_) {
        leftX = startBattleX_;
        hasLeft = true;
    }

    if (hasDoor_) {
        rightX = doorPos_.x;
        hasRight = true;
    } else if (boss) {
        rightX = boss->GetPosition().x + 200.0f;
        hasRight = true;
    }

    if (hasLeft && hasRight) {
        outCenter.x = (leftX + rightX) / 2.0f;
    } else if (hasRight) {
        outCenter.x = rightX - 250.0f;
    } else if (boss) {
        outCenter.x = boss->GetPosition().x - 150.0f;
    } else {
        return false;
    }

    if (hasDoor_ && doorPos_.y > 0.0f) {
        outCenter.y = doorPos_.y - 50.0f;
    } else if (boss) {
        outCenter.y = boss->GetPosition().y - 30.0f;
    } else {
        outCenter.y = 200.0f;
    }

    return true;
}

void BossBattleController::DrawWorld(DragonBoss* boss) const {
    // All door blocks are rendered cleanly and accurately from mapDoorBlocks_
}

void BossBattleController::DrawHUD(DragonBoss* boss, float screenW, float screenH) const {
    if (!boss || boss->IsDead() && phase_ == BossBattlePhase::RoomCleared) return;
    if (phase_ == BossBattlePhase::Waiting) return;

    // Draw Boss HP Bar on top-center of screen
    float barW = screenW * 0.45f;
    float barH = 22.0f;
    float barX = (screenW - barW) * 0.5f;
    float barY = screenH * 0.08f;

    // Boss Name Label
    const char* bossLabel = "BOWSER / DRAGON BOSS";
    int labelSize = (int)(screenH * 0.028f);
    int labelW = MeasureText(bossLabel, labelSize);
    DrawText(bossLabel, (int)((screenW - labelW) * 0.5f), (int)(barY - labelSize - 4.0f), labelSize, GOLD);

    // Outer Background & Border
    DrawRectangle((int)barX - 3, (int)barY - 3, (int)barW + 6, (int)barH + 6, BLACK);
    DrawRectangle((int)barX, (int)barY, (int)barW, (int)barH, Color{ 40, 20, 20, 255 });

    // Health Fill
    float hpRatio = (float)boss->GetHp() / (float)boss->GetMaxHp();
    hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);
    float fillW = barW * hpRatio;

    Color barColor = (hpRatio > 0.5f) ? RED : (hpRatio > 0.25f ? ORANGE : MAROON);
    DrawRectangle((int)barX, (int)barY, (int)fillW, (int)barH, barColor);
    DrawRectangleLines((int)barX - 3, (int)barY - 3, (int)barW + 6, (int)barH + 6, WHITE);
}
