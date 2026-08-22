#pragma once
#include "Game_Objects/Derived_Objects/Projectiles/Projectiles.h"

class Fireball : public Projectile {
private:
    float lifeTimer_ = 0.0f;
    static constexpr float kSpeed = 220.0f;
    static constexpr float kAOERadius = 40.0f;
    static constexpr float kExplodeVisualDuration = 0.2f; // brief lingering flash, damage is instant
    static constexpr float kMaxLifetime = 5.0f;           // despawn if it never hits anything

public:
    // direction: -1.0f = travels left, 1.0f = travels right.
    Fireball(Vector2 startPos, float direction);
    ~Fireball() override = default;

    float GetAOERadius() const override { return kAOERadius; }
    bool CanHurtPlayer(const Player& player) const override;

    void Update(float dt) override;
    void Draw() override;
};