#pragma once
#include "Game Objects/Core Header Files/Characters.h"

class PlayerState;

class Player : public Character {
private:
    PlayerState* state = nullptr;
public:
    ~Player();

    void SetState(PlayerState* Temp);
    void TakeDamage();

    //Inherit from Character.h
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    //Inherit from BaseGameObjects.h
    void Update(float dt) override;

    // render the player's current animation frame at position_,
    // flipped horizontally if facing_ == FacingDirection::Left
    void Draw() override;
};