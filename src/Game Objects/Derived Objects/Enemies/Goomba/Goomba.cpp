#include "Goomba.h"
#include "Game Objects/Interaction Resolve/Visitor.h"
#include "physics/ProximityAI.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"

namespace {
constexpr float kDetectionRadius = 300.0f; // pixels: how close the player must get before the goomba starts chasing
}

Goomba::~Goomba() {}

void Goomba::SetPlayerBody(const physics::PhysicsBody* player) {
    playerBody_ = player;
}

void Goomba::SetCollisionBlocks(const std::vector<Rectangle>* blocks) {
    collisionBlocks_ = blocks;
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
    SyncPhysicsBody();

    physics::InputState input;
    if (playerBody_ && collisionBlocks_) {
        physics::ProximityAI::UpdateAI(physicsBody_, *playerBody_, kDetectionRadius, dt, input, *collisionBlocks_);
    }
    physics::PhysicsEngine::ApplyPhysics(physicsBody_, input, dt);

    if (collisionBlocks_) {
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionBlocks_);
    }

    SyncPhysics();
}

void Goomba::Draw() {
    // Placeholder: no sprite attached yet.
}
