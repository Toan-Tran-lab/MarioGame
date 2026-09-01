#include "InputManager.h"

namespace physics {

    // --- Legacy overload (used by sandbox / AI callers) ---
    static int legacyLastHorizontalDir = 0; // -1 for left, 1 for right

    void InputManager::UpdateInput(InputState& state) {
        bool leftDown = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        bool rightDown = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) legacyLastHorizontalDir = -1;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) legacyLastHorizontalDir = 1;

        if (leftDown && !rightDown) legacyLastHorizontalDir = -1;
        if (rightDown && !leftDown) legacyLastHorizontalDir = 1;
        if (!leftDown && !rightDown) legacyLastHorizontalDir = 0;

        if (leftDown && rightDown) {
            state.moveLeft = (legacyLastHorizontalDir == -1);
            state.moveRight = (legacyLastHorizontalDir == 1);
        } else {
            state.moveLeft = leftDown;
            state.moveRight = rightDown;
        }

        state.moveDown  = IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S);
        state.jumpPressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
        state.jumpHeld    = IsKeyDown(KEY_SPACE)    || IsKeyDown(KEY_W)    || IsKeyDown(KEY_UP);
        state.sprint = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_Z);
        state.shootPressed = IsKeyPressed(KEY_J) || IsKeyPressed(KEY_F);
    }

    // --- Per-player keybinding overload ---
    void InputManager::UpdateInput(InputState& state, const PlayerKeyBindings& bindings) {
        bool leftDown = IsKeyDown(bindings.left);
        bool rightDown = IsKeyDown(bindings.right);

        if (IsKeyPressed(bindings.left)) bindings.lastHorizontalDir = -1;
        if (IsKeyPressed(bindings.right)) bindings.lastHorizontalDir = 1;

        if (leftDown && !rightDown) bindings.lastHorizontalDir = -1;
        if (rightDown && !leftDown) bindings.lastHorizontalDir = 1;
        if (!leftDown && !rightDown) bindings.lastHorizontalDir = 0;

        if (leftDown && rightDown) {
            state.moveLeft = (bindings.lastHorizontalDir == -1);
            state.moveRight = (bindings.lastHorizontalDir == 1);
        } else {
            state.moveLeft = leftDown;
            state.moveRight = rightDown;
        }

        state.moveDown  = IsKeyDown(bindings.down);

        bool jumpMain = IsKeyPressed(bindings.jump);
        bool jumpAlt  = (bindings.jumpAlt != 0) && IsKeyPressed(bindings.jumpAlt);
        state.jumpPressed = jumpMain || jumpAlt;

        bool jumpMainHeld = IsKeyDown(bindings.jump);
        bool jumpAltHeld  = (bindings.jumpAlt != 0) && IsKeyDown(bindings.jumpAlt);
        state.jumpHeld = jumpMainHeld || jumpAltHeld;

        state.sprint = IsKeyDown(bindings.sprint);
        state.shootPressed = IsKeyPressed(bindings.shoot);
    }

} // namespace physics
