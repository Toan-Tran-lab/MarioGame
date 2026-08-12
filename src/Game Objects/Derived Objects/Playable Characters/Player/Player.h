#pragma once
#include "Game Objects/Core Header Files/Characters.h"
#include "world/TileMap.h"
#include "Animations/Animation.h"
#include <vector>

class PlayerState;

class Player : public Character {
private:
    PlayerState* state = nullptr;
    const std::vector<Rectangle>* collisionBlocks_ = nullptr;
    Animation* currentAnimation = nullptr;
    
public:
    ~Player();

    void SetState(PlayerState* Temp);
    void TakeDamage();
    void SetCollisionBlocks(const std::vector<Rectangle>* blocks);
    void SetAnimation(Animation* newAnim);

    // Animation Factory Methods
    virtual Animation* CreatePoseAnimation() = 0;
    virtual Animation* CreateWalkAnimation() = 0;
    virtual Animation* CreateJumpAnimation() = 0;
    virtual Animation* CreateSlideAnimation() = 0;

    //Inherit from Character.h
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    //Inherit from BaseGameObjects.h
    void Update(float dt) override;

    // render the player's current animation frame at position_,
    // flipped horizontally if facing_ == FacingDirection::Left
    void Draw() override;
};