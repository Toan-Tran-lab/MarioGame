#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "World/TileMap.h"
#include "Animations/Animation.h"
#include "physics/InputManager.h"
#include <vector>

class PlayerState;
class BlockGrid;
enum class PowerupType;

class Player : public Character {
private:
    PlayerState* state = nullptr;
    const BlockGrid* collisionGrid_ = nullptr;
    std::vector<Rectangle> dynamicPlatforms_; // moving solid platforms injected each frame
    AnimationState animState;
    bool isDead_ = false;
    bool isSmall_ = true;
    bool isSitting_ = false;
    bool canHitBlock_ = true;
    float prevVelY_ = 0.0f;
    physics::PlayerKeyBindings keyBindings_; // per-player hardware key mapping

public:
    Player();
    ~Player();

    //Attribute Details
    virtual float GetMoveSpeedMultiplier() const { return 1.0f; }
    virtual float GetJumpForce()           const { return -450.0f; }
    virtual float GetGravityMultiplier()   const { return 1.0f; }
    virtual float GetSkidDecel() const { return 0.0f; }

    // Key bindings — call before the first Update() to assign P1 / P2 keys
    void SetKeyBindings(const physics::PlayerKeyBindings& bindings) { keyBindings_ = bindings; }

    //Display Details
    void SetState(PlayerState* Temp);
    void TakeDamage();
    void TakePowerup(PowerupType type);
    void SetCollisionGrid(const BlockGrid* grid);
    // Call each frame before Update() to register moving platform rects (e.g. FlyingBridge).
    // Cleared automatically at the start of the next Update().
    void SetDynamicPlatforms(const std::vector<Rectangle>& platforms);
    void SetAnimation(const Animation* newAnim);

    //Block Hitting Details
    bool CanHitBlock() const { return canHitBlock_; }
    void SetCanHitBlock(bool can) { canHitBlock_ = can; }

    //Dead Details
    bool IsDead() const;
    void SetDead(bool dead);

    //Default State Details
    bool IsSmall() const;
    void SetIsSmall(bool small);

    //Sitting Details
    bool IsSitting() const;
    void SetSitting(bool sitting);

    bool IsProjectileImmune() const;

    // Animation Getters
    virtual const Animation* GetPoseAnimation() const = 0;
    virtual const Animation* GetWalkAnimation() const = 0;
    virtual const Animation* GetJumpAnimation() const = 0;
    virtual const Animation* GetSlideAnimation() const = 0;
    virtual const Animation* GetSitAnimation() const = 0;
    virtual const Animation* GetDieAnimation() const = 0;

    //Inherit from Character.h
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    //Inherit from BaseGameObjects.h
    void Update(float dt) override;

    // render the player's current animation frame at position_,
    // flipped horizontally if facing_ == FacingDirection::Left
    void Draw() override;
};