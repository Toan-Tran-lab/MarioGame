#include "GroundEnemy.h"
#include "physics/ProximityAI.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "World/BlockGrid.h"
#include <cmath>

void GroundEnemy::UpdateBehavior(float dt, physics::InputState& input) {
    if (playerBody_ && collisionGrid_) {
        physics::ProximityAI::UpdateAI(physicsBody_, *playerBody_, GetDetectionRadius(), dt, input, *collisionGrid_);
    }
}

void GroundEnemy::UpdateFacingAndAnim(float dt) {
    if (physicsBody_.velocity.x > 0.0f) facing_ = FacingDirection::Right;
    else if (physicsBody_.velocity.x < 0.0f) facing_ = FacingDirection::Left;
    animState.Update(dt);
}

void GroundEnemy::Update(float dt) {
    SyncPhysicsBody();

    physics::InputState input;
    UpdateBehavior(dt, input);

    physics::PhysicsEngine::ApplyPhysics(physicsBody_, input, dt);

    float prevVelX = physicsBody_.velocity.x;
    if (collisionGrid_) {
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionGrid_);
    }
    PostCollision(prevVelX);

    SyncPhysics();
    UpdateFacingAndAnim(dt);
}