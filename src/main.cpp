#include "raylib.h"
#include "Global/Global.h"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Mario Game");
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
