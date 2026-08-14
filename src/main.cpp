#include "raylib.h"
#include "Global/Global.h"

int main() {
    InitWindow(Global::BASE_WIDTH, Global::BASE_HEIGHT, "Mario Game");
    SetExitKey(0); // Disable ESC auto-exit; our states handle ESC themselves
    ChangeDirectory(GetApplicationDirectory());
    SetTargetFPS(60);

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
