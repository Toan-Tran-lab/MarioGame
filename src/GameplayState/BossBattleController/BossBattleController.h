#pragma once
#include "raylib.h"
#include <vector>
#include <memory>

class Player;
class DragonBoss;

enum class BossBattlePhase {
    Waiting,       // Player has not crossed StartBattle trigger yet
    Intro,         // reDoor shuts, boss prepares/roars
    Fighting,      // Boss fight active, arena locked, HP bar shown
    BossDefeated,  // Boss defeated, death sequence & coin scatter
    RoomCleared    // Door unlocked & open, Mario can pass to reach Princess
};

class BossBattleController {
private:
    BossBattlePhase phase_ = BossBattlePhase::Waiting;

    bool hasBattle_ = false;
    bool hasReDoor_ = false;
    Vector2 reDoorPos_{ 0.0f, 0.0f };

    bool hasDoor_ = false;
    Vector2 doorPos_{ 0.0f, 0.0f };

    bool hasStartBattle_ = false;
    float startBattleX_ = 0.0f;

    float phaseTimer_ = 0.0f;
    static constexpr float kIntroDuration = 1.0f;
    static constexpr float kDefeatedDuration = 2.0f;

public:
    BossBattleController() = default;
    ~BossBattleController() = default;

    void Reset();

    void SetReDoor(Vector2 pos);
    void SetDoor(Vector2 pos);
    void SetStartBattle(float x);

    bool HasBattle() const { return hasBattle_; }
    BossBattlePhase GetPhase() const { return phase_; }

    void Update(float dt, const std::vector<Player*>& activePlayers, DragonBoss* boss);

    // Adds solid barrier collision boxes into dynamic platforms
    void AppendDynamicBarriers(std::vector<Rectangle>& platforms, DragonBoss* boss) const;

    // Checks if camera should be constrained to the boss room arena
    bool GetCameraBounds(float& minWorldX, float& maxWorldX) const;

    void DrawWorld(DragonBoss* boss) const;
    void DrawHUD(DragonBoss* boss, float screenW, float screenH) const;
};
