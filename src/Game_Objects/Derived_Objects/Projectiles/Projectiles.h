#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"

class KoopaShell;
class GroundEnemy;
class Player;

// Lightweight, non-Character moving hazard — deliberately outside the
// Character/CharacterVisitor hierarchy. A manager (GameplayState) checks
// overlap against player/shells/enemies/BlockGrid directly each frame and
// calls these hooks, rather than going through double dispatch.
class Projectile : public GameObject {
protected:
    Vector2 velocity_{0.0f, 0.0f};
    bool exploded_ = false;
    float explodeTimer_ = 0.0f;

public:
    virtual ~Projectile() = default;

    const Vector2& GetVelocity() const { return velocity_; }
    void SetVelocity(const Vector2& v) { velocity_ = v; }
    bool IsExploded() const { return exploded_; }

    virtual void OnHitSolid() { Explode(); }
    virtual void OnHitShell(KoopaShell&) { Explode(); }
    virtual void OnHitEnemy(GroundEnemy&) { Explode(); }

    // Lets a projectile check the player's current immunity before hurting them.
    virtual bool CanHurtPlayer(const Player&) const { return true; }

    // Radius of any AOE effect triggered on explosion. 0 = no AOE.
    virtual float GetAOERadius() const { return 0.0f; }

    // Marks exploded and halts movement. Guarded against double-firing.
    virtual void Explode() {
        if (exploded_) return;
        exploded_ = true;
        velocity_ = { 0.0f, 0.0f };
    }
};