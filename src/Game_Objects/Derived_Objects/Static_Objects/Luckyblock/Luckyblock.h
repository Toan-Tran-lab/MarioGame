#pragma once
#include "Game_Objects/Derived_Objects/Static_Objects/Block/Block.h"
#include "Animations/Animation.h"

class Luckyblock : public Block {
private:
    bool isEmpty_;
    AnimationState animState;

public:
    Luckyblock();
    ~Luckyblock() override;

    bool IsEmpty() const { return isEmpty_; }
    bool IsLucky() const override { return !isEmpty_; }
    
    // Returns true if successfully bumped (wasn't empty), so we can spawn a coin
    bool Bump() override;

    void Update(float dt) override;
    void Draw() override;
};
