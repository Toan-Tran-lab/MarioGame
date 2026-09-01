#include "BuzzyBeetle.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "TextureManager/TextureManager.h"
#include "World/BlockGrid.h"
#include "Global/Global.h"
#include <cmath>

namespace {
constexpr float kFrameDuration = 0.15f;
}

// --- Static animation definitions (Flyweight) ---
static const Animation buzzyWalkAnim("buzzy_walk", 16, 16, 0, 2, {kFrameDuration});
static const Animation buzzyHideAnim("buzzy_hide", 16, 16, 0, 1, {1.0f});

BuzzyBeetle::BuzzyBeetle() {
    animState.SetAnimation(&buzzyWalkAnim);
    facing_ = FacingDirection::Left;
    physicsBody_.aiDirection = -1;
}

BuzzyBeetle::~BuzzyBeetle() {}

// ---------------------------------------------------------------------------
// State helpers
// ---------------------------------------------------------------------------

void BuzzyBeetle::CeilingPatrolLogic(float dt) {
    // No gravity — keep Y locked at spawn height.
    physicsBody_.velocity.y = 0.0f;
    position_.y = spawnPosition_.y;

    // Patrol horizontally at constant speed.
    physicsBody_.velocity.x = physicsBody_.aiDirection * kCeilingSpeed;

    // Detect player below: within kDropDetectionX on X axis.
    if (playerBody_) {
        float beetleCenterX = position_.x + size_.x / 2.0f;
        float playerCenterX = playerBody_->position.x + playerBody_->size.x / 2.0f;
        float dx = std::abs(beetleCenterX - playerCenterX);
        bool playerBelow = playerBody_->position.y > position_.y;

        if (dx <= kDropDetectionX && playerBelow) {
            state_ = BuzzyBeetleState::Dropping;
        }
    }
}

void BuzzyBeetle::DroppingLogic(float dt) {
    // Free-fall with no horizontal movement.
    physicsBody_.velocity.x = 0.0f;

    // Transition to Hiding once we land.
    if (physicsBody_.isGrounded) {
        state_ = BuzzyBeetleState::Hiding;
        hideTimer_ = 0.0f;
        physicsBody_.velocity = {0.0f, 0.0f};
        animState.SetAnimation(&buzzyHideAnim);
    }
}

void BuzzyBeetle::HidingLogic(float dt) {
    physicsBody_.velocity = {0.0f, 0.0f};
    hideTimer_ += dt;

    if (hideTimer_ >= kHideTransitionDuration) {
        // Wake up immediately — start ground patrol.
        state_ = BuzzyBeetleState::GroundPatrol;
        animState.SetAnimation(&buzzyWalkAnim);
    }
}

// ---------------------------------------------------------------------------
// GroundEnemy hooks
// ---------------------------------------------------------------------------

void BuzzyBeetle::UpdateBehavior(float dt, physics::InputState& input) {
    switch (state_) {
        case BuzzyBeetleState::CeilingPatrol:
            input.ignorePhysics = true;
            CeilingPatrolLogic(dt);
            break;

        case BuzzyBeetleState::Dropping:
            input.ignorePhysics = true;
            DroppingLogic(dt);
            break;

        case BuzzyBeetleState::Hiding:
            input.ignorePhysics = true;
            HidingLogic(dt);
            break;

        case BuzzyBeetleState::GroundPatrol:
            GroundEnemy::UpdateBehavior(dt, input);
            break;
    }
}

void BuzzyBeetle::UpdateFacingAndAnim(float dt) {
    if (state_ == BuzzyBeetleState::Hiding) {
        animState.Update(dt);
        return;
    }
    GroundEnemy::UpdateFacingAndAnim(dt);
}

// ---------------------------------------------------------------------------
// Main Update
// ---------------------------------------------------------------------------

void BuzzyBeetle::Update(float dt) {
    // Record spawn position on first update.
    if (!spawnRecorded_) {
        spawnPosition_ = position_;
        spawnRecorded_ = true;
    }

    GroundEnemy::Update(dt);

    // Post-pipeline: keep ceiling beetle locked to its Y.
    if (state_ == BuzzyBeetleState::CeilingPatrol) {
        if (std::abs(physicsBody_.velocity.x) < 0.1f) {
            physicsBody_.aiDirection *= -1;
        }
        position_.y = spawnPosition_.y;
        physicsBody_.position.y = spawnPosition_.y;
        physicsBody_.velocity.y = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// Interaction
// ---------------------------------------------------------------------------

void BuzzyBeetle::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void BuzzyBeetle::Draw() {
    if (!TextureManager::Has("buzzy_walk")) {
        TextureManager::Load("buzzy_walk", "assets/textures/BuzzyBeetle/walk/enemies.png");
    }
    if (!TextureManager::Has("buzzy_hide")) {
        TextureManager::Load("buzzy_hide", "assets/textures/BuzzyBeetle/hide/enemies.png");
    }

    Vector2 drawPos = { position_.x, position_.y };

    switch (state_) {
        case BuzzyBeetleState::CeilingPatrol: {
            // Draw upside-down: use negative srcRect.height for vertical flip.
            const Animation* anim = animState.GetAnimation();
            if (anim && TextureManager::Has(anim->textureKey)) {
                int frameIndex = animState.GetCurrentFrameIndex();
                Rectangle srcRect = TextureManager::GetSourceRect(
                    anim->textureKey, anim->tileW, anim->tileH, frameIndex);

                // Horizontal flip for facing direction.
                if (facing_ == FacingDirection::Left) {
                    srcRect.width = -srcRect.width;
                }
                // Vertical flip for ceiling (Raylib convention: negative height).
                srcRect.height = -srcRect.height;

                Rectangle destRect = { drawPos.x, drawPos.y, size_.x, size_.y };
                DrawTexturePro(TextureManager::Get(anim->textureKey),
                               srcRect, destRect, {0, 0}, 0.0f, WHITE);
            }
            break;
        }

        case BuzzyBeetleState::Hiding: {
            // Draw hide sprite like KoopaShell.
            Texture2D& tex = TextureManager::Get("buzzy_hide");
            int tileW = 16;
            int tileH = 16;
            int frameIndex = (hideTimer_ < kHideTransitionDuration) ? 1 : 0;

            Rectangle srcRect = TextureManager::GetSourceRect("buzzy_hide", tileW, tileH, frameIndex);
            Rectangle destRect = { drawPos.x, drawPos.y, size_.x, size_.y };
            DrawTexturePro(tex, srcRect, destRect, {0, 0}, 0.0f, WHITE);
            break;
        }

        case BuzzyBeetleState::Dropping:
        case BuzzyBeetleState::GroundPatrol:
        default:
            animState.Draw(drawPos, facing_, size_);
            break;
    }
}