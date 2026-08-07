#include "InputManager.h"

namespace physics {

    void InputManager::UpdateInput(InputState& state) {
        // Map WASD or Arrow keys to left/right
        state.moveLeft = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        state.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
        
        // Map Space or W to jump. We check for a pressed event for initial jumping
        state.jumpPressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W);
        // We check for a down event for variable jump height (hover mechanics)
        state.jumpHeld = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W);
    }

} // namespace physics
