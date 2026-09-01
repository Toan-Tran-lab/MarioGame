#pragma once
#include "Game_Objects/Derived_Objects/Projectiles/Projectiles.h"
#include "Animations/Animation.h"

class PlayerFireball : public Projectile {
private:
    float lifeTimer_ = 0.0f;
    float explosionTimer_ = 0.0f;
    AnimationState animState;
    static constexpr float kSpeed = 400.0f;
    static constexpr float kMaxLifetime = 3.0f;

public:
    PlayerFireball(Vector2 startPos, float direction);
    ~PlayerFireball() override = default;

    bool CanHurtPlayer(const Player&) const override { return false; } // Can't hurt Mario

    void Update(float dt) override;
    void Draw() override;
    
    void Explode() override;
    void OnHitShell(KoopaShell& shell) override;
    void OnHitEnemy(GroundEnemy& enemy) override;
};
