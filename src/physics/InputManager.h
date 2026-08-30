#pragma once
#include <raylib.h>

namespace physics {

    // Abstract game events produced by hardware keys
    struct InputState {
        bool moveLeft = false;
        bool moveRight = false;
        bool moveDown = false;
        bool jumpPressed = false;
        bool jumpHeld = false;
        bool sprint = false;
        bool ignorePhysics = false; // Bypasses horizontal acceleration and friction for simple AI
    };

    // Hardware key mapping for a single player. Each Player instance stores its own.
    struct PlayerKeyBindings {
        int left   = KEY_A;
        int right  = KEY_D;
        int down   = KEY_S;
        int jump   = KEY_W;
        int jumpAlt = KEY_SPACE; // Secondary jump key (0 = none)
        int sprint = KEY_LEFT_SHIFT;

        // Internal tracker for simultaneous input (recent-key-wins)
        mutable int lastHorizontalDir = 0; // -1 for left, 1 for right, 0 for none
    };

    class InputManager {
    public:
        // Polls using default bindings (legacy: used by sandbox / AI callers)
        static void UpdateInput(InputState& state);

        // Polls using the given per-player key bindings
        static void UpdateInput(InputState& state, const PlayerKeyBindings& bindings);
    };

} // namespace physics
