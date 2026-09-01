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
    hasReDoor_ = false;
    reDoorPos_ = { 0.0f, 0.0f };
    hasDoor_ = false;
    doorPos_ = { 0.0f, 0.0f };
    hasStartBattle_ = false;
    startBattleX_ = 0.0f;
    phaseTimer_ = 0.0f;
}

void BossBattleController::SetReDoor(Vector2 pos) {
    reDoorPos_ = pos;
    hasReDoor_ = true;
    hasBattle_ = true;
}

void BossBattleController::SetDoor(Vector2 pos) {
    doorPos_ = pos;
    hasDoor_ = true;
    hasBattle_ = true;
}

void BossBattleController::SetStartBattle(float x) {
    startBattleX_ = x;
    hasStartBattle_ = true;
    hasBattle_ = true;
}

void BossBattleController::Update(float dt, const std::vector<Player*>& activePlayers, DragonBoss* boss) {
    if (!hasBattle_ && !boss) return;

    switch (phase_) {
        case BossBattlePhase::Waiting: {
            bool trigger = false;
            if (hasStartBattle_) {
                for (Player* p : activePlayers) {
                    if (p && !p->IsDead() && p->GetPosition().x >= startBattleX_) {
                        trigger = true;
                        break;
                    }
                }
            } else if (boss && boss->IsActive()) {
                for (Player* p : activePlayers) {
                    if (p && !p->IsDead() && p->GetPosition().x >= boss->GetPosition().x - 400.0f) {
                        trigger = true;
                        break;
                    }
                }
            }

            if (trigger) {
                phase_ = BossBattlePhase::Intro;
                phaseTimer_ = kIntroDuration;
                AudioManager::PlayBGM(AudioKey::BGM_DRAGON_BOSS);
            }
            break;
        }

        case BossBattlePhase::Intro: {
            phaseTimer_ -= dt;
            if (phaseTimer_ <= 0.0f) {
                phase_ = BossBattlePhase::Fighting;
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
    float doorW = 16.0f * Global::GAME_SCALE;
    float doorH = 48.0f * Global::GAME_SCALE;

    // 1. reDoor: When closed (Intro, Fighting, BossDefeated, RoomCleared), prevents retreating left
    if (hasReDoor_ && phase_ != BossBattlePhase::Waiting) {
        platforms.push_back(Rectangle{ reDoorPos_.x, reDoorPos_.y, doorW, doorH });
    }

    // 2. Door: Locked until RoomCleared
    if (hasDoor_ && phase_ != BossBattlePhase::RoomCleared) {
        platforms.push_back(Rectangle{ doorPos_.x, doorPos_.y, doorW, doorH });
    }
}

bool BossBattleController::GetCameraBounds(float& minWorldX, float& maxWorldX) const {
    if (phase_ == BossBattlePhase::Fighting || phase_ == BossBattlePhase::Intro || phase_ == BossBattlePhase::BossDefeated) {
        if (hasReDoor_ && hasDoor_) {
            minWorldX = reDoorPos_.x;
            maxWorldX = doorPos_.x;
            return true;
        }
    }
    return false;
}

void BossBattleController::DrawWorld(DragonBoss* boss) const {
    float doorW = 16.0f * Global::GAME_SCALE;
    float doorH = 48.0f * Global::GAME_SCALE;

    // --- Draw reDoor (Entrance Gate) ---
    if (hasReDoor_) {
        bool isClosed = (phase_ != BossBattlePhase::Waiting);
        if (isClosed) {
            // Closed iron portcullis gate
            DrawRectangle((int)reDoorPos_.x, (int)reDoorPos_.y, (int)doorW, (int)doorH, Color{ 50, 45, 45, 255 });
            for (float y = reDoorPos_.y; y < reDoorPos_.y + doorH; y += 16.0f * Global::GAME_SCALE) {
                DrawRectangle((int)reDoorPos_.x + 4, (int)y + 4, (int)doorW - 8, (int)(16.0f * Global::GAME_SCALE) - 8, Color{ 80, 75, 75, 255 });
            }
            for (float x = reDoorPos_.x + 8; x < reDoorPos_.x + doorW; x += 12) {
                DrawRectangle((int)x, (int)reDoorPos_.y, 4, (int)doorH, Color{ 130, 120, 120, 255 });
            }
            DrawRectangleLines((int)reDoorPos_.x, (int)reDoorPos_.y, (int)doorW, (int)doorH, Color{ 30, 25, 25, 255 });
        } else {
            // Open gate frame outline
            DrawRectangleLines((int)reDoorPos_.x, (int)reDoorPos_.y, (int)doorW, (int)doorH, Color{ 90, 80, 80, 150 });
        }
    }

    // --- Draw Door (Exit Door) ---
    if (hasDoor_) {
        bool isOpen = (phase_ == BossBattlePhase::RoomCleared);
        if (!isOpen) {
            // Locked wooden/iron door with golden lock
            DrawRectangle((int)doorPos_.x, (int)doorPos_.y, (int)doorW, (int)doorH, Color{ 90, 50, 25, 255 });
            DrawRectangle((int)doorPos_.x + 4, (int)doorPos_.y + 4, (int)doorW - 8, (int)doorH - 8, Color{ 120, 70, 35, 255 });
            // Iron bands
            DrawRectangle((int)doorPos_.x + 2, (int)doorPos_.y + 20, (int)doorW - 4, 10, Color{ 60, 60, 65, 255 });
            DrawRectangle((int)doorPos_.x + 2, (int)doorPos_.y + (int)doorH - 30, (int)doorW - 4, 10, Color{ 60, 60, 65, 255 });
            // Golden lock
            DrawRectangle((int)doorPos_.x + (int)(doorW / 2) - 8, (int)doorPos_.y + (int)(doorH / 2) - 8, 16, 16, Color{ 230, 190, 40, 255 });
            DrawRectangleLines((int)doorPos_.x, (int)doorPos_.y, (int)doorW, (int)doorH, Color{ 40, 20, 10, 255 });
        } else {
            // Open illuminated doorway
            DrawRectangle((int)doorPos_.x, (int)doorPos_.y, (int)doorW, (int)doorH, Color{ 25, 15, 35, 220 });
            DrawRectangleLines((int)doorPos_.x, (int)doorPos_.y, (int)doorW, (int)doorH, Color{ 220, 190, 50, 255 });
        }
    }
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
