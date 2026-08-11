#pragma once
#include "raylib.h"
#include "Vector2Ops.h"

enum class FacingDirection {
    Left,
    Right
};

struct CollisionInfo {
    Vector2 normal;       // e.g. (0, -1) = other object hit this one from below
    Vector2 penetration;
};

class GameObject {
public:
    // Virtual destructor as needed for a pure abstract base class
    virtual ~GameObject() = default;

    // Pure virtual interface: the "contract" every GameObject must fulfill
    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;

    // Shared, non-virtual accessors (common to all objects in the world (static items, 
    // playable characters, enemiers, etc.))
    const Vector2& GetPosition() const { return position_; }
    void SetPosition(const Vector2& pos) { position_ = pos; }

    bool IsActive() const { return isActive_; }
    void SetActive(bool active) { isActive_ = active; }

    virtual void OnCollision(GameObject& other, const CollisionInfo& info) = 0;

protected:
    // Determine the position of the object in the world
    Vector2 position_{ 0.0f, 0.0f };

private:
    // For object pooling / despawn logic
    bool isActive_ = true;
};