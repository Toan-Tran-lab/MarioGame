#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"

class Coin : public GameObject {
private:
    float animTimer_ = 0.0f;
    int animFrame_ = 0;
    float velocityY_ = 0.0f;
    float basePositionY_ = 0.0f;
    float settledTimer_ = 0.0f;
    bool hasLanded_ = false;
    bool isPopping_ = false;
    bool awardsScoreOnCollect_ = true;
public:
    void SetPopping(bool popping, float initialVelocity) {
        isPopping_ = popping;
        velocityY_ = initialVelocity;
        basePositionY_ = position_.y;
    }
    void SetAwardsScoreOnCollect(bool awards) { awardsScoreOnCollect_ = awards; }
    bool AwardsScoreOnCollect() const { return awardsScoreOnCollect_; }

    void Update(float dt) override;
    void Draw() override;
};