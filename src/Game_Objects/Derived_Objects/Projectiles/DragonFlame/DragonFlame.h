#pragma once
#include "Game_Objects/Derived_Objects/Projectiles/Projectiles.h"
#include "Animations/Animation.h"

class DragonFlame : public Projectile {
private:
    float lifeTimer_ = 0.0f;
    static constexpr float kSpeed = 240.0f;
    static constexpr float kMaxLifetime = 6.0f;
    static constexpr float kExplodeDuration = 0.25f;

    AnimationState animState_;
    FacingDirection facing_ = FacingDirection::Left;

public:
    DragonFlame(Vector2 startPos, float directionX);
    ~DragonFlame() override = default;

    bool CanHurtPlayer(const Player& player) const override;

    void Update(float dt) override;
    void Draw() override;
};
