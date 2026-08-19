#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"

class BlockGrid;

// A solid platform that moves horizontally and reverses direction
// when it hits a map border or a solid tile. Mario can stand on it.
class FlyingBridge : public GameObject {
private:
    float velocityX_ = 100.0f;     // horizontal speed (px/s), sign = direction
    float leftBound_ = 0.0f;      // patrol left limit (world pixels)
    float rightBound_ = 0.0f;     // patrol right limit (world pixels)
    const BlockGrid* blockGrid_ = nullptr;

public:
    FlyingBridge() {
        // Default size: 3 tiles wide x 1 tile tall (at 48px per tile)
        size_ = { 144.0f, 48.0f };
    }

    void SetVelocityX(float vx) { velocityX_ = vx; }
    float GetVelocityX() const { return velocityX_; }

    void SetPatrolBounds(float left, float right) {
        leftBound_ = left;
        rightBound_ = right;
    }

    void SetBlockGrid(const BlockGrid* grid) { blockGrid_ = grid; }

    Rectangle GetRect() const {
        return { position_.x, position_.y, size_.x, size_.y };
    }

    void Update(float dt) override;
    void Draw() override;
};
