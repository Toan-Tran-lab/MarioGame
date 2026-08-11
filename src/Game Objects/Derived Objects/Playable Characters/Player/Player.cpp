#include "Player.h"
#include "Game Objects/Interaction Resolve/PlayerInteraction.h"
#include "PlayerState.h"
#include "physics/InputManager.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"

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

void Player::SetCollisionBlocks(const std::vector<Rectangle>* blocks) {
    collisionBlocks_ = blocks;
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
}

void Player::Draw() {}