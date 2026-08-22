#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"

class Block : public GameObject {
protected:
    float basePositionY_ = 0.0f;
    float bumpVelocity_ = 0.0f;
    float bumpOffsetY_ = 0.0f;
    bool isBumping_ = false;
    bool isSolid_ = true;

public:
    virtual ~Block() = default;
    
    // Returns true if block triggered a special event (e.g. lucky block spawn)
    virtual bool Bump() {
        if (!isBumping_) {
            isBumping_ = true;
            bumpVelocity_ = -160.0f;
            bumpOffsetY_ = 0.0f;
        }
        return false;
    }
    
    virtual bool IsLucky() const { return false; }

    float GetBumpOffsetY() const { return bumpOffsetY_; }
    
    void SetPosition(const Vector2& pos) override {
        position_ = pos;
        basePositionY_ = pos.y;
    }
    
    Rectangle GetRect() const {
        return { position_.x, position_.y, size_.x, size_.y };
    }

    bool IsSolid() const { return isSolid_; }

    void Update(float dt) override {
        if (isBumping_) {
            bumpOffsetY_ += bumpVelocity_ * dt;
            bumpVelocity_ += 1000.0f * dt;
            if (bumpOffsetY_ >= 0.0f && bumpVelocity_ > 0.0f) {
                bumpOffsetY_ = 0.0f;
                bumpVelocity_ = 0.0f;
                isBumping_ = false;
            }
            position_.y = basePositionY_ + bumpOffsetY_;
        }
    }
    
    void Draw() override {}
};

class TerrainBlock : public Block {
public:
    TerrainBlock() { isSolid_ = true; }
    // Inherits empty Update and Draw
};
