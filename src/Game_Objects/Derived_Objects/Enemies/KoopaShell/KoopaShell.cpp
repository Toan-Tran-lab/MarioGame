#include <cmath>
#include "KoopaShell.h"
#include "Game_Objects/Interaction_Resolve/KoopaShellInteraction.h"
#include "World/BlockGrid.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"

namespace {
constexpr float kFrameDuration   = 0.15f;  // seconds per walk frame
constexpr float kSlideSpeed      = 300.0f; // pixels per second when sliding
constexpr float kSpinDuration    = 0.10f;  // seconds per spin frame
}

// --- Static animation definitions (Flyweight) ---
static const Animation koopaWalkAnim("koopa_walk", 16, 24, 0, 2, {kFrameDuration});
static const Animation koopaUpsideDownAnim("koopa_upsidedown", 70, 106, 0, 1, {1.0f}); // 70x106

KoopaShell::KoopaShell() {
    animState.SetAnimation(&koopaWalkAnim);
}

KoopaShell::~KoopaShell() {}

void KoopaShell::TriggerUpsideDownDeath(bool hitFromLeft) {
    GroundEnemy::TriggerUpsideDownDeath(hitFromLeft);
    animState.SetAnimation(&koopaUpsideDownAnim);
}

void KoopaShell::UpdateBehavior(float dt, physics::InputState& input) {
    if (state_ == KoopaShellState::Walking) {
        GroundEnemy::UpdateBehavior(dt, input); // default chase
    } else if (state_ == KoopaShellState::Sliding) {
        input.ignorePhysics = true;
        physicsBody_.velocity.x = (physicsBody_.velocity.x >= 0.0f) ? kSlideSpeed : -kSlideSpeed;
        spinTimer_ += dt;
        if (spinTimer_ >= kSpinDuration) {
            spinTimer_ = 0.0f;
            spinFrame_ = (spinFrame_ + 1) % 4;
        }
    }
    // Hiding: leave input default — physics (gravity) still runs, matches original.
}

void KoopaShell::PostCollision(float prevVelX) {
    if (state_ == KoopaShellState::Sliding &&
        std::abs(physicsBody_.velocity.x) < 1.0f && std::abs(prevVelX) > 1.0f) {
        physicsBody_.velocity.x = -prevVelX; // bounced off a wall
    }
}

void KoopaShell::UpdateFacingAndAnim(float dt) {
    if (state_ == KoopaShellState::Walking) {
        GroundEnemy::UpdateFacingAndAnim(dt); // default facing + anim
    }
    // Hiding/Sliding: no facing/anim change — spin frame is handled separately in Draw().
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
    if (state_ != KoopaShellState::Sliding) return; // only slides initiate
    ShellInteraction visitor(*this);
    other.AcceptInteract(visitor);
}

void KoopaShell::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void KoopaShell::Update(float dt) {
    if (state_ == KoopaShellState::Hiding) {
        hidingTimer_ += dt;
    }
    GroundEnemy::Update(dt);
}

void KoopaShell::Draw() {
    if (!TextureManager::Has("koopa_walk")) {
        TextureManager::Load("koopa_walk", "assets/textures/Koopa/walk/enemies.png");
    }
    if (!TextureManager::Has("koopa_hide")) {
        TextureManager::Load("koopa_hide", "assets/textures/Koopa/hide/enemies.png");
    }
    if (!TextureManager::Has("koopa_upsidedown")) {
        TextureManager::Load("koopa_upsidedown", "assets/textures/Koopa/dead/enemies.png");
    }

    Vector2 drawPos = { position_.x, position_.y };

    if (upsideDownDead_) {
        animState.Draw(drawPos, facing_, size_);
    } else if (state_ == KoopaShellState::Walking) {
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
