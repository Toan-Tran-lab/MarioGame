#include <raylib.h>
#include <vector>
// Include our physics module headers using the include path
#include "physics/PhysicsBody.h"
#include "physics/InputManager.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "physics/ProximityAI.h"

using namespace physics;

int main() {
    // 1. Initialization
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Role 4 Physics & AI Sandbox");
    SetTargetFPS(60);

    // 2. Set up our isolated test entities
    PhysicsBody player;
    player.position = { 100, 300 };
    player.size = { 32, 32 };
    
    PhysicsBody enemy;
    enemy.position = { 500, 400 };
    enemy.size = { 32, 32 };

    // 3. Set up the solid environment
    std::vector<Rectangle> blocks = {
        { 0, 500, 800, 100 },       // Floor
        { 200, 400, 100, 100 },     // Wall / Step
        { 400, 350, 100, 20 },      // Floating Platform
        { -50, 0, 50, 600 },        // Left boundary
        { 800, 0, 50, 600 }         // Right boundary
    };

    InputState inputState;

    // Main game loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- UPDATE ---
        InputManager::UpdateInput(inputState);

        // Apply Player Physics
        PhysicsEngine::ApplyPhysics(player, inputState, dt);
        
        // Update Enemy AI to generate inputs
        InputState enemyInput;
        // ProximityAI::UpdateAI(enemy, player, 300.0f, dt, enemyInput, blocks);

        // Apply Enemy Physics using the generated AI inputs
        PhysicsEngine::ApplyPhysics(enemy, enemyInput, dt);

        // Map Collisions
        CollisionSystem::ResolveMapCollisions(player, blocks);
        CollisionSystem::ResolveMapCollisions(enemy, blocks);

        // Test Stomp Mechanics (Mocking Player vs Enemy collision)
        CollisionInfo pveCol = CollisionSystem::GetCollisionInfo(player.GetRect(), enemy.GetRect());
        if (pveCol.side == CollisionSide::BOTTOM) {
            // Player stomped enemy
            enemy.position.y = 1000; // "kill" enemy by moving it offscreen
            player.velocity.y = PhysicsEngine::JUMP_FORCE * 0.5f; // small bounce
        } else if (pveCol.side == CollisionSide::LEFT || pveCol.side == CollisionSide::RIGHT || pveCol.side == CollisionSide::TOP) {
            // Player took damage
            player.position.x = 100; // Reset player
            player.position.y = 300;
        }

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw blocks
        for (const auto& block : blocks) {
            DrawRectangleRec(block, DARKGRAY);
        }

        // Draw entities
        DrawRectangleRec(player.GetRect(), BLUE);
        DrawRectangleRec(enemy.GetRect(), RED);
        
        // Draw AI Detection Radius
        DrawCircleLines((int)enemy.position.x + 16, (int)enemy.position.y + 16, 300.0f, Fade(RED, 0.3f));

        DrawText("Physics Sandbox: Use Arrows/WASD to Move, Space/W to Jump, Shift/Z to Sprint.", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    // De-Initialization
    CloseWindow();
    return 0;
}
