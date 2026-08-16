#include <cmath>
#include "Goomba.h"
#include "Game Objects/Interaction Resolve/Visitor.h"
#include "physics/ProximityAI.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "TextureManager/TextureManager.h"
#include "World/BlockGrid.h"

namespace {
constexpr float kDetectionRadius = 300.0f; // pixels: how close the player must get before the goomba starts chasing
constexpr float kFrameDuration   = 0.15f;  // seconds per walk frame (roughly 6-7 FPS)
// "+100" popup constants
constexpr float kPopupSpeed      = 60.0f;  // pixels per second, floats upward
constexpr float kPopupFontSize   = 24;
}

// --- Static animation definitions (Flyweight) ---
static const Animation goombaWalkAnim("goomba_walk", 16, 16, 0, 2, {kFrameDuration});
static const Animation goombaDeadAnim("goomba_dead", 16, 16, 0, 1, {1.0f}); // single frame, no loop needed

Goomba::Goomba() {
    animState.SetAnimation(&goombaWalkAnim);
}

Goomba::~Goomba() {}

void Goomba::SetPlayerBody(const physics::PhysicsBody* player) {
    playerBody_ = player;
}

void Goomba::SetCollisionGrid(const BlockGrid* grid) {
    collisionGrid_ = grid;
}

void Goomba::Stomp() {
    if (state_ == GoombaState::Dying) return; // already dying, ignore
    state_ = GoombaState::Dying;
    dyingTimer_ = 0.0f;
    animState.SetAnimation(&goombaDeadAnim);
    // Stop all movement
    physicsBody_.velocity = { 0.0f, 0.0f };
}

void Goomba::InteractWith(Character& other) {
    // Interactions are player-initiated: the player drives resolution through
    // AcceptInteract() and a visitor. A goomba has nothing to do on its own.
    (void)other;
}

void Goomba::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void Goomba::Update(float dt) {
    if (state_ == GoombaState::Dying) {
        dyingTimer_ += dt;
        if (dyingTimer_ >= kDyingDuration) {
            SetActive(false);
        }
        return; // No AI or physics while dying
    }

    // --- Normal alive behaviour ---
    SyncPhysicsBody();

    physics::InputState input;
    if (playerBody_ && collisionGrid_) {
        physics::ProximityAI::UpdateAI(physicsBody_, *playerBody_, kDetectionRadius, dt, input, *collisionGrid_);
    }
    physics::PhysicsEngine::ApplyPhysics(physicsBody_, input, dt);

    if (collisionGrid_) {
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionGrid_);
    }

    SyncPhysics();

    // Face the direction the goomba is moving
    if (physicsBody_.velocity.x > 0.0f) facing_ = FacingDirection::Right;
    else if (physicsBody_.velocity.x < 0.0f) facing_ = FacingDirection::Left;

    // Advance walk animation
    animState.Update(dt);
}

void Goomba::Draw() {
    // Ensure textures are loaded
    if (!TextureManager::Has("goomba_walk")) {
        TextureManager::Load("goomba_walk", "assets/textures/Goomba/walk/enemies.png");
    }
    if (!TextureManager::Has("goomba_dead")) {
        TextureManager::Load("goomba_dead", "assets/textures/Goomba/dead/enemies.png");
    }

    Vector2 drawPos = { position_.x, position_.y };

    if (state_ == GoombaState::Dying) {
        // Draw the squished dead sprite (centered vertically at the bottom half)
        // The dead goomba is flat, so draw it at the bottom of the tile
        float deadH = size_.y * 0.5f; // squished to half height
        Vector2 deadPos = { drawPos.x, drawPos.y + size_.y - deadH };
        Vector2 deadSize = { size_.x, deadH };
        animState.Draw(deadPos, FacingDirection::Right, deadSize);

        // Draw "+100" popup floating upward over time
        float popupOffsetY = -kPopupSpeed * dyingTimer_;
        float alpha = 1.0f - (dyingTimer_ / kDyingDuration); // fade out
        int fontSize = kPopupFontSize;
        const char* popupText = "+100";
        int textW = MeasureText(popupText, fontSize);
        int px = (int)(drawPos.x + size_.x / 2.0f - textW / 2.0f);
        int py = (int)(drawPos.y + popupOffsetY);
        DrawText(popupText, px, py, fontSize, Fade(YELLOW, alpha));
    } else {
        animState.Draw(drawPos, facing_, size_);
    }
}
