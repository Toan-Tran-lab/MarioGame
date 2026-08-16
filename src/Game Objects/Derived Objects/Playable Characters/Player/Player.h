#pragma once
#include "Game Objects/Core Header Files/Characters.h"
#include "world/TileMap.h"
#include "Animations/Animation.h"
#include <vector>

class PlayerState;
enum class PowerupType;

class Player : public Character {
private:
    PlayerState* state = nullptr;
    const std::vector<Rectangle>* collisionBlocks_ = nullptr;
    AnimationState animState;
    bool isDead_ = false;
    bool isSmall_ = true;
    bool isSitting_ = false;

public:
    Player();
    ~Player();

    void SetState(PlayerState* Temp);
    void TakeDamage();
    void TakePowerup(PowerupType type);
    void SetCollisionBlocks(const std::vector<Rectangle>* blocks);
    void SetAnimation(const Animation* newAnim);

    bool IsDead() const;
    void SetDead(bool dead);

    bool IsSmall() const;
    void SetIsSmall(bool small);

    bool IsSitting() const;
    void SetSitting(bool sitting);

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