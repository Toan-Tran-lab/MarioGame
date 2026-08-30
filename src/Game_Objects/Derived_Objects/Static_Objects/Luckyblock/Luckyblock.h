#pragma once
#include "Game_Objects/Derived_Objects/Static_Objects/Block/Block.h"
#include "Animations/Animation.h"

enum class LuckyContents { Coin, Mushroom };

class Luckyblock : public Block {
private:
    bool isEmpty_;
    AnimationState animState;
    LuckyContents lastContents_ = LuckyContents::Coin; // set fresh each successful Bump()

public:
    Luckyblock();
    ~Luckyblock() override;

    bool IsEmpty() const { return isEmpty_; }
    bool IsLucky() const override { return !isEmpty_; }

    bool Bump() override;
    LuckyContents GetLastContents() const { return lastContents_; }

    void Update(float dt) override;
    void Draw() override;
};