#pragma once
#include "raylib.h"

inline Vector2 operator+(const Vector2& a, const Vector2& b) {
    return { a.x + b.x, a.y + b.y };
}
inline Vector2 operator*(const Vector2& v, float scalar) {
    return { v.x * scalar, v.y * scalar };
}