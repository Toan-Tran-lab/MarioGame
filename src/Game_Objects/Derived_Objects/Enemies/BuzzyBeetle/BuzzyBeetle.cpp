#include "BuzzyBeetle.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "TextureManager/TextureManager.h"
#include "World/BlockGrid.h"

namespace {
constexpr float kFrameDuration = 0.15f;
constexpr float kPopupSpeed    = 60.0f;
constexpr float kPopupFontSize = 24;
}

// --- Static animation definitions (Flyweight) ---
// Placeholder texture keys — swap to real asset paths once art is ready.
static const Animation buzzyWalkAnim("buzzy_walk", 16, 16, 0, 2, {kFrameDuration});
static const Animation buzzyFlippedAnim("buzzy_flipped", 16, 16, 0, 1, {1.0f});

BuzzyBeetle::BuzzyBeetle() {
    animState.SetAnimation(&buzzyWalkAnim);
}

BuzzyBeetle::~BuzzyBeetle() {}

void BuzzyBeetle::Defeat() {
    if (state_ == BuzzyBeetleState::Dying) return; // already defeated, ignore
    state_ = BuzzyBeetleState::Dying;
    flipped_ = true;
    dyingTimer_ = 0.0f;
    animState.SetAnimation(&buzzyFlippedAnim);
    physicsBody_.velocity = { 0.0f, 0.0f };
}

void BuzzyBeetle::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void BuzzyBeetle::Update(float dt) {
    if (state_ == BuzzyBeetleState::Dying) {
        dyingTimer_ += dt;
        if (dyingTimer_ >= kDyingDuration) SetActive(false);
        return; // no AI or physics while flipped/dying
    }
    GroundEnemy::Update(dt);
}

void BuzzyBeetle::Draw() {
    if (!TextureManager::Has("buzzy_walk")) {
        TextureManager::Load("buzzy_walk", "assets/textures/BuzzyBeetle/walk/enemies.png");
    }
    if (!TextureManager::Has("buzzy_flipped")) {
        TextureManager::Load("buzzy_flipped", "assets/textures/BuzzyBeetle/flipped/enemies.png");
    }

    Vector2 drawPos = { position_.x, position_.y };
    animState.Draw(drawPos, facing_, size_);

    if (state_ == BuzzyBeetleState::Dying) {
        float popupOffsetY = -kPopupSpeed * dyingTimer_;
        float alpha = 1.0f - (dyingTimer_ / kDyingDuration);
        int fontSize = kPopupFontSize;
        const char* popupText = "+100";
        int textW = MeasureText(popupText, fontSize);
        int px = (int)(drawPos.x + size_.x / 2.0f - textW / 2.0f);
        int py = (int)(drawPos.y + popupOffsetY);
        DrawText(popupText, px, py, fontSize, Fade(YELLOW, alpha));
    }
}