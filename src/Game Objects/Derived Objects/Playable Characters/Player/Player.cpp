#include "Player.h"
#include "Game Objects/Interaction Resolve/PlayerInteraction.h"
#include "PlayerState.h"
#include "physics/InputManager.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "World/BlockGrid.h"
#include <cmath>

Player::Player() : state(new SmallState()) {}

Player::~Player() {
    if (state != nullptr) delete state;
    state = nullptr;
}
void Player::SetState(PlayerState* Temp) {
    if (state) state->Exit(*this);
    delete state;
    state = Temp;
    if(state) state->Enter(*this);
}

void Player::TakeDamage() {
    if (state) state->OnHit(*this);
}

void Player::TakePowerup(PowerupType type) {
    if (state) state->OnPowerup(*this, type);
}

void Player::SetDead(bool dead) {
    isDead_ = dead;
}

bool Player::IsDead() const {
    return isDead_;
}

bool Player::IsSmall() const {
    return isSmall_;
}

void Player::SetIsSmall(bool small) {
    isSmall_ = small;
}

bool Player::IsSitting() const {
    return isSitting_;
}

void Player::SetSitting(bool sitting) {
    isSitting_ = sitting;
}

void Player::SetCollisionGrid(const BlockGrid* grid) {
    collisionGrid_ = grid;
}

void Player::SetAnimation(const Animation* newAnim) {
    animState.SetAnimation(newAnim);
}

void Player::InteractWith(Character& other) {
    PlayerInteraction visitor(*this);
    other.AcceptInteract(visitor);
}

void Player::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void Player::Update(float dt) {
    if (state) state->UpdateState(*this, dt);

    SyncPhysicsBody();

    physics::InputState input;
    physics::InputManager::UpdateInput(input);
    
    // Handle dead state
    if (isDead_) {
        if (animState.GetAnimation() != GetDieAnimation()) {
            SetAnimation(GetDieAnimation());
            // Small jump when dying
            physicsBody_.velocity.y = -400.0f;
            physicsBody_.velocity.x = 0;
        }
        
        // Apply physics (gravity) but no input
        physics::InputState noInput;
        noInput.ignorePhysics = false; // still want gravity
        physics::PhysicsEngine::ApplyPhysics(physicsBody_, noInput, dt);
        // Do NOT resolve map collisions so player falls out of bounds
        SyncPhysics();
        
        animState.Update(dt);
        return;
    }

    // Handle sit state
    if (IsGrounded() && input.moveDown) {
        SetSitting(true);
        input.moveLeft = false;
        input.moveRight = false;
        input.jumpPressed = false;
        input.jumpHeld = false;
        physicsBody_.velocity.x = 0; // stop moving
    } else {
        SetSitting(false);
    }

    physics::PhysicsEngine::ApplyPhysics(physicsBody_, input, dt);

    float curVelY = physicsBody_.velocity.y;
    if (IsGrounded() || (prevVelY_ >= 0.0f && curVelY < 0.0f)) {
        canHitBlock_ = true;
    }
    prevVelY_ = curVelY;

    if (collisionGrid_) {
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionGrid_);
    }

    SyncPhysics();

    // Auto-update facing based on input (allows moonwalking/sliding)
    if (input.moveLeft && !input.moveRight) facing_ = FacingDirection::Left;
    else if (input.moveRight && !input.moveLeft) facing_ = FacingDirection::Right;

    // Determine Animation State
    // Very basic logic: if not grounded -> Jump; if sliding -> Slide; if moving -> Walk; else -> Pose
    // For now, let's keep it simple
    if (isSitting_) {
        if (animState.GetAnimation() != GetSitAnimation()) {
            SetAnimation(GetSitAnimation());
        }
    } else if (!IsGrounded()) {
        if (animState.GetAnimation() != GetJumpAnimation()) {
            SetAnimation(GetJumpAnimation());
        }
    } else if (std::abs(physicsBody_.velocity.x) > 0.1f) {
        // Simple slide check: moving one way, pressing the other
        bool onlyLeft = input.moveLeft && !input.moveRight;
        bool onlyRight = input.moveRight && !input.moveLeft;
        bool slidingLeft = (physicsBody_.velocity.x > 0 && onlyLeft);
        bool slidingRight = (physicsBody_.velocity.x < 0 && onlyRight);
        if (slidingLeft || slidingRight) {
            if (animState.GetAnimation() != GetSlideAnimation()) {
                SetAnimation(GetSlideAnimation());
            }
        } else {
            if (animState.GetAnimation() != GetWalkAnimation()) {
                SetAnimation(GetWalkAnimation());
            }
        }
    } else {
        if (animState.GetAnimation() != GetPoseAnimation()) {
            SetAnimation(GetPoseAnimation());
        }
    }

    animState.Update(dt);
}

void Player::Draw() {
    Vector2 drawPos = { position_.x, position_.y };
    animState.Draw(drawPos, facing_, size_);
}