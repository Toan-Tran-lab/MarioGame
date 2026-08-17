#include <cmath>
#include "KoopaShell.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "physics/ProximityAI.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "World/BlockGrid.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"

namespace {
constexpr float kDetectionRadius = 300.0f; // pixels: how close the player must get before it starts chasing
constexpr float kFrameDuration   = 0.15f;  // seconds per walk frame
constexpr float kSlideSpeed      = 300.0f; // pixels per second when sliding
constexpr float kSpinDuration    = 0.10f;  // seconds per spin frame
}

// --- Static animation definitions (Flyweight) ---
static const Animation koopaWalkAnim("koopa_walk", 16, 24, 0, 2, {kFrameDuration});

KoopaShell::KoopaShell() {
    animState.SetAnimation(&koopaWalkAnim);
}

KoopaShell::~KoopaShell() {}

void KoopaShell::SetPlayerBody(const physics::PhysicsBody* player) {
    playerBody_ = player;
}

void KoopaShell::SetCollisionGrid(const BlockGrid* grid) {
    collisionGrid_ = grid;
}

void KoopaShell::Stomp() {
    if (state_ == KoopaShellState::Walking || state_ == KoopaShellState::Sliding) {
        state_ = KoopaShellState::Hiding;
        hidingTimer_ = 0.0f;
        // Adjust size from 16x24 to 16x16
        float oldBottom = position_.y + size_.y;
        size_.y = 16.0f * Global::GAME_SCALE;
        position_.y = oldBottom - size_.y; // keep bottom aligned
        SyncPhysicsBody();
        // Stop movement
        physicsBody_.velocity = { 0.0f, 0.0f };
    }
}

void KoopaShell::Kick(int dir) {
    if (state_ == KoopaShellState::Hiding || state_ == KoopaShellState::Sliding) {
        state_ = KoopaShellState::Sliding;
        physicsBody_.velocity.x = dir * kSlideSpeed;
        spinTimer_ = 0.0f;
        spinFrame_ = 0;
    }
}

void KoopaShell::InteractWith(Character& other) {
    // Interactions are player-initiated. 
    (void)other;
}

void KoopaShell::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void KoopaShell::Update(float dt) {
    SyncPhysicsBody();

    physics::InputState input;
    if (state_ == KoopaShellState::Walking) {
        if (playerBody_ && collisionGrid_) {
            physics::ProximityAI::UpdateAI(physicsBody_, *playerBody_, kDetectionRadius, dt, input, *collisionGrid_);
        }
    } else if (state_ == KoopaShellState::Sliding) {
        // Bypass speed caps/friction
        input.ignorePhysics = true;
        physicsBody_.velocity.x = (physicsBody_.velocity.x >= 0.0f) ? kSlideSpeed : -kSlideSpeed;
        
        // Spin animation update
        spinTimer_ += dt;
        if (spinTimer_ >= kSpinDuration) {
            spinTimer_ = 0.0f;
            spinFrame_ = (spinFrame_ + 1) % 4; // 4 spin frames: 0, 2, 3, 4 (indexed as 0,1,2,3 for simplicity later)
        }
    } else if (state_ == KoopaShellState::Hiding) {
        hidingTimer_ += dt;
    }

    physics::PhysicsEngine::ApplyPhysics(physicsBody_, input, dt);

    if (collisionGrid_) {
        float prevVelX = physicsBody_.velocity.x;
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionGrid_);
        
        if (state_ == KoopaShellState::Sliding) {
            // If horizontal velocity changed significantly, it means we hit a wall
            if (std::abs(physicsBody_.velocity.x) < 1.0f && std::abs(prevVelX) > 1.0f) {
                physicsBody_.velocity.x = -prevVelX; // Bounce!
            }
        }
    }

    SyncPhysics();

    if (state_ == KoopaShellState::Walking) {
        // Face the direction it is moving
        if (physicsBody_.velocity.x > 0.0f) facing_ = FacingDirection::Right;
        else if (physicsBody_.velocity.x < 0.0f) facing_ = FacingDirection::Left;

        animState.Update(dt);
    }
}

void KoopaShell::Draw() {
    if (!TextureManager::Has("koopa_walk")) {
        TextureManager::Load("koopa_walk", "assets/textures/Koopa/walk/enemies.png");
    }
    if (!TextureManager::Has("koopa_hide")) {
        TextureManager::Load("koopa_hide", "assets/textures/Koopa/hide/enemies.png");
    }

    Vector2 drawPos = { position_.x, position_.y };

    if (state_ == KoopaShellState::Walking) {
        animState.Draw(drawPos, facing_, size_);
    } else {
        // Hiding or Sliding uses the koopa_hide texture
        Texture2D& tex = TextureManager::Get("koopa_hide");
        int tileW = 16;
        int tileH = 16;
        int frameIndex = 0; // Default: fully hidden
        
        if (state_ == KoopaShellState::Hiding) {
            if (hidingTimer_ < kHideTransitionDuration) {
                frameIndex = 1; // getting in, legs visible
            } else {
                frameIndex = 0; // fully hidden
            }
        } else if (state_ == KoopaShellState::Sliding) {
            // Spin animation frames to make a full circle: 0 (flat), 3 (45 deg), 2 (90 deg), 4 (135 deg)
            int spinFrames[4] = {0, 3, 2, 4};
            frameIndex = spinFrames[spinFrame_];
        }

        Rectangle srcRect = TextureManager::GetSourceRect("koopa_hide", tileW, tileH, frameIndex);
        Rectangle destRect = { drawPos.x, drawPos.y, size_.x, size_.y };
        DrawTexturePro(tex, srcRect, destRect, {0, 0}, 0.0f, WHITE);
    }
}
