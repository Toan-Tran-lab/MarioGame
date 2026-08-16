#include "Mushroom.h"
#include "Game Objects/Interaction Resolve/Visitor.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "Global/Global.h"
#include <cmath>

static const Animation mushroomAnim("mushroom", 16, 16, 0, 1, {1.0f});

Mushroom::Mushroom() {
    animState.SetAnimation(&mushroomAnim);
    size_ = {Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE};
    physicsBody_.size = {Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE};
    physicsBody_.aiDirection = 1;
}

void Mushroom::Update(float dt) {
    if (!active) return;
    
    physics::InputState noInput;
    noInput.ignorePhysics = true; // Let the physics engine know it's a simple entity
    
    // Let AI or manual logic set velocity.x, ApplyPhysics will handle x/y movement
    physicsBody_.velocity.x = physicsBody_.aiDirection * 100.0f;

    physics::PhysicsEngine::ApplyPhysics(physicsBody_, noInput, dt);

    if (collisionBlocks_) {
        // Resolve collisions
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionBlocks_);
        
        // Bounce off walls if x velocity was zeroed by collision
        if (std::abs(physicsBody_.velocity.x) < 0.1f) {
            physicsBody_.aiDirection *= -1;
            physicsBody_.velocity.x = physicsBody_.aiDirection * 100.0f;
        }
        
        if (physicsBody_.isGrounded) {
            physicsBody_.velocity.y = 0;
        }
    }

    SyncPhysics();
    animState.Update(dt);
}

void Mushroom::Draw() {
    if (!active) return;
    Vector2 drawPos = { std::round(position_.x), std::round(position_.y) };
    animState.Draw(drawPos, facing_, size_);
}

void Mushroom::InteractWith(Character& other) {
    // Mushroom doesn't initiate interactions, Player does.
}

void Mushroom::AcceptInteract(CharacterVisitor& visitor) {
    visitor.Visit(*this);
}
