#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"

// Fire entity acts as an instant-death hazard.
class Fire : public GameObject {
public:
    Fire() {
        size_ = { 48.0f, 48.0f }; // Scaled size (16x16 LDtk units * 3 scale)
    }

    Rectangle GetRect() const {
        return { position_.x, position_.y, size_.x, size_.y };
    }

    void Update(float dt) override {
        // Static hazard, no specific update logic needed unless animated
    }

    void Draw() override {
        // Left empty - typically the visual fire is drawn by the LDtk TileMap
    }
};
