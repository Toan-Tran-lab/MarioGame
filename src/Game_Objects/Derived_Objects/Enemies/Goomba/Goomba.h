#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Ground_Enemy/GroundEnemy.h"
#include "Animations/Animation.h"
#include <vector>

enum class GoombaState { Alive, Dying };

class Goomba : public GroundEnemy {
private:
    GoombaState state_ = GoombaState::Alive;
    float dyingTimer_ = 0.0f;
    static constexpr float kDyingDuration = 0.5f; // seconds before disappearing

public:
    Goomba();
    ~Goomba();

    // Called when stomped by the player — switches to Dying state
    void Stomp();
    bool IsDying() const { return state_ == GoombaState::Dying; }
    void TriggerUpsideDownDeath(bool hitFromLeft) override;

    void AcceptInteract(CharacterVisitor& other) override;
    void Update(float dt) override;
    void Draw() override;
};
