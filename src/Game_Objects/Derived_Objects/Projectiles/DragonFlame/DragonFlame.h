#pragma once
#include "Game_Objects/Derived_Objects/Projectiles/Projectiles.h"
#include "Animations/Animation.h"

class DragonFlame : public Projectile {
private:
    float growth_ = 0.0f;
    Vector2 mouthPos_{ 0.0f, 0.0f };
    float directionX_ = -1.0f;

    AnimationState animState_;
    FacingDirection facing_ = FacingDirection::Left;

public:
    DragonFlame(Vector2 mouthPos, float directionX);
    ~DragonFlame() override = default;

    void SetMouthAnchor(Vector2 mouthPos, float directionX, float growth);
    void StopBreathing();

    bool CanHurtPlayer(const Player& player) const override;

    void Update(float dt) override;
    void Draw() override;
};
