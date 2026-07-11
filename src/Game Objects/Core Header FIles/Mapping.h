#pragma once

// For positioning within the game world
struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    Vector2() = default;
    Vector2(float X, float Y) : x(X), y(Y) {}

    Vector2 operator+(const Vector2& rhs) const { return { x + rhs.x, y + rhs.y }; }
    Vector2 operator*(float scalar)        const { return { x * scalar, y * scalar }; }
};