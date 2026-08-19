#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Ground_Enemy/GroundEnemy.h"
#include "Animations/Animation.h"

enum class BuzzyBeetleState { Alive, Dying };

class BuzzyBeetle : public GroundEnemy {
private:
    BuzzyBeetleState state_ = BuzzyBeetleState::Alive;
    float dyingTimer_ = 0.0f;
    static constexpr float kDyingDuration = 0.6f; // slightly longer than Goomba's, to sell the flip

    // True once defeated by a shell. Drives the "on its back" draw state.
    // Flip condition: ONLY a sliding-shell hit sets this — player stomps
    // never do, since the beetle is immune to the player entirely.
    bool flipped_ = false;

public:
    BuzzyBeetle();
    ~BuzzyBeetle();

    // Called only by ShellInteraction::Visit(BuzzyBeetle&) on a sliding-shell hit.
    void Defeat();
    bool IsDefeated() const { return state_ == BuzzyBeetleState::Dying; }
    bool IsFlipped() const { return flipped_; }

    // Reserved for later: spikes/lava will call something like TakeHazardDamage()
    // once the hazard system exists. Not implemented yet — noted, not forgotten.

    void AcceptInteract(CharacterVisitor& other) override;
    void Update(float dt) override;
    void Draw() override;
};