#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"

// GoalPipe acts as a level checkpoint or level completion trigger.
// When the player touches it, they automatically slide down into it,
// which eventually triggers the Level Complete screen.
class GoalPipe : public GameObject {
private:
    bool isTriggered_ = false;
    bool isAnimationComplete_ = false;
    float slideTimer_ = 0.0f;
    float maxSlideDuration_ = 1.5f;  // Seconds to slide down
    float slideSpeed_ = 48.0f;       // Pixels per second to slide down

    // Player position handling during animation
    Vector2 playerStartPos_ = {0,0};
    Vector2 playerCurrentPos_ = {0,0};

public:
    GoalPipe() {
        // Size of the goal pipe (scaled) - 2 blocks high to cover both upper and lower pipe tiles
        size_ = { 48.0f, 96.0f };
    }

    Rectangle GetRect() const {
        return { position_.x, position_.y, size_.x, size_.y };
    }

    // Starts the pipe entry animation for the player
    void Trigger(const Vector2& playerPos) {
        isTriggered_ = true;
        slideTimer_ = 0.0f;
        // Since it's a horizontal pipe, Mario walks into it directly.
        playerStartPos_ = playerPos; 
        playerCurrentPos_ = playerStartPos_;
    }

    bool IsTriggered() const { return isTriggered_; }
    bool IsAnimationComplete() const { return isAnimationComplete_; }

    // Returns the player's position during the slide
    Vector2 GetPlayerAnimPos() const { return playerCurrentPos_; }

    void Update(float dt) override;
    void Draw() override;
};
