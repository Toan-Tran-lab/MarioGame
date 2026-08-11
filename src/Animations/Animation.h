#pragma once
#include "raylib.h"
#include "Game Objects/Core Header Files/BaseGameObjects.h"
#include <string>

class Animation {
protected:
    float animTimer = 0.0f;
    int currentFrame = 0;
    
public:
    virtual ~Animation() = default;
    
    // Virtual methods for state updates and drawing
    virtual void Update(float dt) = 0;
    virtual void Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) = 0;
};

// Posing / Idle animation
class PoseAnimation : public Animation {
public:
    void Update(float dt) override;
    void Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) override;
};

// Walking animation
class WalkAnimation : public Animation {
public:
    void Update(float dt) override;
    void Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) override;
};

// Jumping animation
class JumpAnimation : public Animation {
public:
    void Update(float dt) override;
    void Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) override;
};

// Sliding / Skidding animation
class SlideAnimation : public Animation {
public:
    void Update(float dt) override;
    void Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) override;
};
