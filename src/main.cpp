#include "raylib.h"
#include "Global/Global.h"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1092, 768, "Mario Game");
    SetExitKey(0); // Disable ESC auto-exit; our states handle ESC themselves
    ChangeDirectory(GetApplicationDirectory());
    // SetTargetFPS(60);

    Global::Init();

    while (!WindowShouldClose() && !Global::shouldExit) {
        float dt = GetFrameTime();
        Global::gameStateManager->Update(dt);

        BeginDrawing();
        Global::gameStateManager->Draw();
        EndDrawing();
    }

    Global::Cleanup();
    CloseWindow();
    return 0;
}
