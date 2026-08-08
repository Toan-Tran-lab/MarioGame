#pragma once
#include <raylib.h>

namespace physics {

    // Abstract game events produced by hardware keys
    struct InputState {
        bool moveLeft = false;
        bool moveRight = false;
        bool jumpPressed = false;
        bool jumpHeld = false;
        bool sprint = false;
    };

    class InputManager {
    public:
        // Polls the keyboard and updates the provided input state
        static void UpdateInput(InputState& state);
    };

} // namespace physics
