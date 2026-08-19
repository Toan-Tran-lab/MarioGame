#pragma once
#include "src/Game_Objects/Core_Header/BaseGameObjects.h"
#include "src/Game_Objects/Derived_Objects/Enemies/Ground_Enemy/GroundEnemy.h"
#include "src/Game_Objects/Derived_Objects/Enemies/KoopaShell/KoopaShell.h"
#include "src/Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"

class Projectile : public GameObject {
protected:
    Vector2 velocity_{0.0f, 0.0f};
    bool exploded_ = false;
public:
    virtual void OnHitSolid() {}          // touched map geometry
    virtual void OnHitShell(KoopaShell&) {}
    virtual void OnHitEnemy(GroundEnemy&) {} // hit any ground enemy (future: other bosses' minions)
    virtual bool CanHurtPlayer(const Player&) const { return true; } // lets Fireball check invuln states
    virtual void Explode() { exploded_ = true; }
    bool IsExploded() const { return exploded_; }
};