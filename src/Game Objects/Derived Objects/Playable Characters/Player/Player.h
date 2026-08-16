#pragma once
#include "Game Objects/Core Header Files/Characters.h"
#include "World/TileMap.h"
#include "Animations/Animation.h"
#include <vector>

class PlayerState;

class Player : public Character {
private:
    PlayerState* state = nullptr;
    const BlockGrid* collisionGrid_ = nullptr;
    AnimationState animState;
    bool isDead_ = false;
<<<<<<< HEAD
=======
    bool isSmall_ = true;
    bool isSitting_ = false;
    bool canHitBlock_ = true;
    float prevVelY_ = 0.0f;
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c

public:
    Player();
    ~Player();

    void SetState(PlayerState* Temp);
    void TakeDamage();
<<<<<<< HEAD
    void SetCollisionBlocks(const std::vector<Rectangle>* blocks);
=======
    void TakePowerup(PowerupType type);
    void SetCollisionGrid(const BlockGrid* grid);
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c
    void SetAnimation(const Animation* newAnim);

    bool CanHitBlock() const { return canHitBlock_; }
    void SetCanHitBlock(bool can) { canHitBlock_ = can; }

    bool IsDead() const;
    void SetDead(bool dead);

    // Animation Getters
    virtual const Animation* GetPoseAnimation() const = 0;
    virtual const Animation* GetWalkAnimation() const = 0;
    virtual const Animation* GetJumpAnimation() const = 0;
    virtual const Animation* GetSlideAnimation() const = 0;

    //Inherit from Character.h
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    //Inherit from BaseGameObjects.h
    void Update(float dt) override;

    // render the player's current animation frame at position_,
    // flipped horizontally if facing_ == FacingDirection::Left
    void Draw() override;
};