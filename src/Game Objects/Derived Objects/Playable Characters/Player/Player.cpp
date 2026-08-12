#include "Player.h"
#include "Game Objects/Interaction Resolve/PlayerInteraction.h"
#include "PlayerState.h"
#include "physics/InputManager.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include <cmath>

Player::Player() : state(new SmallState()) {}

Player::~Player() {
    if (state != nullptr) delete state;
    state = nullptr;
    if (currentAnimation != nullptr) delete currentAnimation;
    currentAnimation = nullptr;
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

void Player::SetDead(bool dead) {
    isDead_ = dead;
}

bool Player::IsDead() const {
    return isDead_;
}

void Player::SetCollisionBlocks(const std::vector<Rectangle>* blocks) {
    collisionBlocks_ = blocks;
}

void Player::SetAnimation(Animation* newAnim) {
    if (currentAnimation) delete currentAnimation;
    currentAnimation = newAnim;
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
    physics::PhysicsEngine::ApplyPhysics(physicsBody_, input, dt);

    if (collisionBlocks_) {
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionBlocks_);
    }

    SyncPhysics();

    // Auto-update facing based on input (allows moonwalking/sliding)
    if (input.moveLeft && !input.moveRight) facing_ = FacingDirection::Left;
    else if (input.moveRight && !input.moveLeft) facing_ = FacingDirection::Right;

    // Determine Animation State
    // Very basic logic: if not grounded -> Jump; if sliding -> Slide; if moving -> Walk; else -> Pose
    // For now, let's keep it simple
    if (!grounded_) {
        if (!dynamic_cast<JumpAnimation*>(currentAnimation)) {
            SetAnimation(CreateJumpAnimation());
        }
    } else if (std::abs(velocity_.x) > 0.1f) {
        // Simple slide check: moving one way, pressing the other
        bool slidingLeft = (velocity_.x > 0 && input.moveLeft);
        bool slidingRight = (velocity_.x < 0 && input.moveRight);
        if (slidingLeft || slidingRight) {
            if (!dynamic_cast<SlideAnimation*>(currentAnimation)) {
                SetAnimation(CreateSlideAnimation());
            }
        } else {
            if (!dynamic_cast<WalkAnimation*>(currentAnimation)) {
                SetAnimation(CreateWalkAnimation());
            }
        }
    } else {
        if (!dynamic_cast<PoseAnimation*>(currentAnimation)) {
            SetAnimation(CreatePoseAnimation());
        }
    }

    if (currentAnimation) {
        currentAnimation->Update(dt);
    }
}

void Player::Draw() {
    if (currentAnimation) {
        currentAnimation->Draw(position_, facing_, size_);
    }
}