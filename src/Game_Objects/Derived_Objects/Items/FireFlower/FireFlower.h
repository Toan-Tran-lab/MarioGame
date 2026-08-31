#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"

class BlockGrid;

class FireFlower : public Character {
private:
    AnimationState animState;

public:
    FireFlower();

    void Spawn(Vector2 pos) { SetPosition(pos); SetActive(true); }

    void Update(float dt) override;
    void Draw() override;
    
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& visitor) override;
};
