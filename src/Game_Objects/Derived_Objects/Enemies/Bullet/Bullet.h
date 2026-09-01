#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"

class Bullet : public Character {
private:
    AnimationState animState;
    float direction_ = -1.0f; // -1.0f = travels left, 1.0f = travels right
    static constexpr float kSpeed = 280.0f;

public:
    // direction: -1.0f = travels left, 1.0f = travels right
    Bullet(Vector2 startPos, float direction);
    ~Bullet() override = default;

    float GetDirection() const { return direction_; }
    void Stomp() { SetActive(false); }
    void OnHitSolid() { SetActive(false); }

    Rectangle GetRect() const {
        float insetX = 4.0f;
        float insetY = 6.0f;
        return Rectangle{ position_.x + insetX, position_.y + insetY, size_.x - 2.0f * insetX, size_.y - 2.0f * insetY };
    }

    void InteractWith(Character& other) override { (void)other; }
    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;
};
