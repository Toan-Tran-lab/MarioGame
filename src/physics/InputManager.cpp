#include "InputManager.h"

namespace physics {

    static int lastHorizontalPress = 0; // -1 for left, 1 for right

    void InputManager::UpdateInput(InputState& state) {
        // Map WASD or Arrow keys to left/right
        bool leftDown = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        bool rightDown = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
        
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) lastHorizontalPress = -1;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) lastHorizontalPress = 1;

        if (leftDown && !rightDown) lastHorizontalPress = -1;
        if (rightDown && !leftDown) lastHorizontalPress = 1;
        if (!leftDown && !rightDown) lastHorizontalPress = 0;

        if (leftDown && rightDown) {
            state.moveLeft = (lastHorizontalPress == -1);
            state.moveRight = (lastHorizontalPress == 1);
        } else {
            state.moveLeft = leftDown;
            state.moveRight = rightDown;
        }
        state.moveLeft = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        state.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
        state.moveDown = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
        
        // Map Space or W to jump. We check for a pressed event for initial jumping
        state.jumpPressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W);
        // We check for a down event for variable jump height (hover mechanics)
        state.jumpHeld = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W);
        
        // Map Left Shift or Z to sprint
        state.sprint = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_Z);
    }

} // namespace physics
