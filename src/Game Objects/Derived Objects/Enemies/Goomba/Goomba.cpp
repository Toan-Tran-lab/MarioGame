#include <cmath>
#include "Goomba.h"
#include "Game Objects/Interaction Resolve/Visitor.h"
#include "physics/ProximityAI.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "TextureManager/TextureManager.h"

namespace {
constexpr float kDetectionRadius = 300.0f; // pixels: how close the player must get before the goomba starts chasing
constexpr float kFrameDuration = 0.15f;    // seconds per walk frame (roughly 6-7 FPS)

// Walk frames inside assets/textures/enemies-3.png. The cells are 16x16 but
// NOT on a regular grid (they are arranged diagonally), so they cannot be
// indexed with TextureManager::GetSourceRect.
const Rectangle kGoombaFrames[3] = {
    { 0.0f,  8.0f, 16.0f, 16.0f },
    { 18.0f, 26.0f, 16.0f, 16.0f },
    { 36.0f, 44.0f, 16.0f, 16.0f },
};
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

    // Face the direction the goomba is moving
    if (physicsBody_.velocity.x > 0.0f) facing_ = FacingDirection::Right;
    else if (physicsBody_.velocity.x < 0.0f) facing_ = FacingDirection::Left;

    // Advance walk animation
    goombaAnimTimer_ += dt;
    if (goombaAnimTimer_ >= kFrameDuration) {
        goombaAnimTimer_ = 0.0f;
        goombaFrame_ = (goombaFrame_ + 1) % 3;
    }
}

void Goomba::Draw() {
    if (!TextureManager::Has("goomba")) {
        TextureManager::Load("goomba", "assets/textures/enemies-3.png");
        return;
    }

    Rectangle src = kGoombaFrames[goombaFrame_];
    if (facing_ == FacingDirection::Left) src.width = -src.width;

    Rectangle dest = { std::round(position_.x), std::round(position_.y), size_.x, size_.y };
    DrawTexturePro(TextureManager::Get("goomba"), src, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
}
