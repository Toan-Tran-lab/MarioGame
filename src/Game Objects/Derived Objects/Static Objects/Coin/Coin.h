#pragma once
#include "Game Objects/Core Header Files/BaseGameObjects.h"

class Coin : public GameObject {
private:
    float animTimer_ = 0.0f;
    int animFrame_ = 0;
    float velocityY_ = 0.0f;
    float basePositionY_ = 0.0f;
    bool isPopping_ = false;

public:
    void SetPopping(bool popping, float initialVelocity) {
        isPopping_ = popping;
        velocityY_ = initialVelocity;
        basePositionY_ = position_.y;
    }

    void Update(float dt) override;
    void Draw() override;
};