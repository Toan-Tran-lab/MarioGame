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
    bool hasDoor_ = false;
    Vector2 doorPos_{ 0.0f, 0.0f };

    bool hasStartBattle_ = false;
    Vector2 startBattlePos_{ 0.0f, 0.0f };
    float startBattleX_ = 0.0f;

    float phaseTimer_ = 0.0f;
    bool introRoarStarted_ = false;

public:
    static constexpr float kIntroDuration = 3.6f;
    static constexpr float kDefeatedDuration = 2.0f;

    BossBattleController() = default;
    ~BossBattleController() = default;

    void Reset();

    void SetDoor(Vector2 pos);
    void SetStartBattle(Vector2 pos);
    void SetStartBattle(float x);

    void BeginIntro(const std::vector<Player*>& activePlayers = {});
    void StartBattle(const std::vector<Player*>& activePlayers = {});
    void TeleportPlayers(const std::vector<Player*>& activePlayers);

    bool HasBattle() const { return hasBattle_; }
    bool HasDoor() const { return hasDoor_; }
    bool HasStartBattle() const { return hasStartBattle_; }
    Vector2 GetDoorPos() const { return doorPos_; }
    Vector2 GetStartBattlePos() const { return startBattlePos_; }
    BossBattlePhase GetPhase() const { return phase_; }
    float GetPhaseTimer() const { return phaseTimer_; }

    void Update(float dt, const std::vector<Player*>& activePlayers, DragonBoss* boss);

    // Adds solid barrier collision boxes into dynamic platforms
    void AppendDynamicBarriers(std::vector<Rectangle>& platforms, DragonBoss* boss) const;

    // Checks if camera should be constrained to the boss room arena
    bool GetCameraBounds(float& minWorldX, float& maxWorldX) const;

    // Gets the center coordinate of the boss arena room for camera centering
    bool GetRoomCenter(Vector2& outCenter, DragonBoss* boss) const;

    void DrawWorld(DragonBoss* boss) const;
    void DrawHUD(DragonBoss* boss, float screenW, float screenH) const;
};
