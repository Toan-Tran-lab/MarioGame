#include "Starman.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "World/BlockGrid.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"
#include <cmath>

static const Animation starmanAnim("starman", 116, 114, 0, 1, {1.0f});

Starman::Starman() {
    animState.SetAnimation(&starmanAnim);
    size_ = {Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE};
    physicsBody_.size = {Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE};
    physicsBody_.aiDirection = 1;
    SetActive(false);
}

void Starman::Update(float dt) {
    if (!IsActive()) return;

    if (!physicsSynced_) {
        SyncPhysicsBody();
        physicsSynced_ = true;
    }
    
    physics::InputState noInput;
    noInput.ignorePhysics = true; 
    
    physicsBody_.velocity.x = physicsBody_.aiDirection * kSpeedX;

    physics::PhysicsEngine::ApplyPhysics(physicsBody_, noInput, dt);

    if (collisionGrid_) {
        physics::CollisionSystem::ResolveMapCollisions(physicsBody_, *collisionGrid_);
        
        // Bounce off walls
        if (std::abs(physicsBody_.velocity.x) < 0.1f) {
            physicsBody_.aiDirection *= -1;
            physicsBody_.velocity.x = physicsBody_.aiDirection * kSpeedX;
        }
        
        // Bounce off ground
        if (physicsBody_.isGrounded) {
            physicsBody_.velocity.y = kBounceVelocity;
            physicsBody_.isGrounded = false;
        }
    }

    SyncPhysics();
    animState.Update(dt);
}

void Starman::Draw() {
    if (!IsActive()) return;
    if (!TextureManager::Has("starman")) {
        TextureManager::Load("starman", "assets/textures/Starman/Starman.png");
    }
    Vector2 drawPos = { position_.x, position_.y };
    animState.Draw(drawPos, facing_, size_);
}

void Starman::InteractWith(Character& other) {
    // Starman doesn't initiate interactions, Player does.
}

void Starman::AcceptInteract(CharacterVisitor& visitor) {
    visitor.Visit(*this);
}
