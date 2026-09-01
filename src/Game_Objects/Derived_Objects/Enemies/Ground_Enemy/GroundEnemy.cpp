#include "GroundEnemy.h"
#include "physics/ProximityAI.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "World/BlockGrid.h"
#include <cmath>

void GroundEnemy::UpdateBehavior(float dt, physics::InputState& input) {
    if (collisionGrid_) {
        physics::ProximityAI::UpdateAI(physicsBody_, playerBody_, GetDetectionRadius(), dt, input, *collisionGrid_);
    }
}

void GroundEnemy::UpdateFacingAndAnim(float dt) {
    if (physicsBody_.velocity.x > 0.0f) facing_ = FacingDirection::Right;
    else if (physicsBody_.velocity.x < 0.0f) facing_ = FacingDirection::Left;
    animState.Update(dt);
}

void GroundEnemy::TriggerUpsideDownDeath(bool hitFromLeft) {
    upsideDownDead_ = true;
    physicsBody_.velocity.x = hitFromLeft ? 150.0f : -150.0f; // Fly away from impact
    physicsBody_.velocity.y = -400.0f; // Bounce up
}

void GroundEnemy::Update(float dt) {
    if (upsideDownDead_) {
        // Only apply gravity and update position without map collisions
        physicsBody_.velocity.y += 1800.0f * dt;
        position_.x += physicsBody_.velocity.x * dt;
        position_.y += physicsBody_.velocity.y * dt;
        animState.Update(dt);
        // Despawn when it falls way off screen
        if (position_.y > 1000.0f) { // Arbitrary off-screen threshold
            SetActive(false);
        }
        return;
    }

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