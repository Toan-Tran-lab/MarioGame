#pragma once
#include "raylib.h"
#include <vector>

class DragonBoss;
class Player;

enum class DragonAction {
    Idle,
    Walk,
    Jump,
    Fire,
    Scream
};

class DragonBossBrain {
private:
    float decisionTimer_ = 0.0f;
    int attackCounter_ = 0;

    Player* primaryTarget_ = nullptr;
    Player* secondaryTarget_ = nullptr;

    bool isPincerSituation_ = false;

    void EvaluatePlayers(DragonBoss& boss, const std::vector<Player*>& players);

public:
    DragonBossBrain() = default;

    void Update(DragonBoss& boss, const std::vector<Player*>& players, float dt);

    DragonAction DecideNextAction(DragonBoss& boss, const std::vector<Player*>& players);

    Player* GetPrimaryTarget() const { return primaryTarget_; }
    bool IsPincerSituation() const { return isPincerSituation_; }
};
