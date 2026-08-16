#pragma once
#include <raylib.h>

namespace physics {

    // A minimal, decoupled structure for physics entities
    struct PhysicsBody {
        Vector2 position = {0.0f, 0.0f};
        Vector2 velocity = {0.0f, 0.0f};
        Vector2 size = {32.0f, 32.0f};
        bool isGrounded = false;
        bool hitCeiling = false;
        Rectangle hitCeilingRect = {0,0,0,0};
        float jumpBufferTimer = 0.0f; // Remembers jump inputs for a few frames
        int aiDirection = -1; // -1 for left, 1 for right
        
        // Helper to get the bounding box for this body
        Rectangle GetRect() const {
            return { position.x, position.y, size.x, size.y };
        }
    };

} // namespace physics
