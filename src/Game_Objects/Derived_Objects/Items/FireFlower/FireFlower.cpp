#include "FireFlower.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"
#include <algorithm>

static const Animation fireflowerAnim("fireflower", 120, 114, 0, 1, {1.0f});

FireFlower::FireFlower() {
    animState.SetAnimation(&fireflowerAnim);
    size_ = {Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE};
    // No physics needed, it just sits there
    SetActive(false);
}

void FireFlower::StartEmerging(float blockTopY) {
    blockOriginY_ = blockTopY;
    emergeTargetY_ = blockTopY - size_.y;
    position_.y = blockTopY;
    isEmerging_ = true;
    SetActive(true);
}

void FireFlower::Update(float dt) { 
    if (!IsActive()) return;
    if (isEmerging_) {
        position_.y -= kEmergeSpeed * dt;
        if (position_.y <= emergeTargetY_) {
            position_.y = emergeTargetY_;
            isEmerging_ = false;
        }
    }
    animState.Update(dt);
}

void FireFlower::Draw() {
    if (!IsActive()) return;
    
    if (!TextureManager::Has("fireflower")) {
        TextureManager::Load("fireflower", "assets/textures/FireFlower/FireFlower.png");
    }

    Vector2 drawPos = { position_.x, position_.y };
    if (isEmerging_) {
        float visibleH = blockOriginY_ - position_.y;
        float fraction = std::clamp(visibleH / size_.y, 0.0f, 1.0f);
        if (fraction > 0.0f) {
            animState.DrawCropped(drawPos, FacingDirection::Right, size_, fraction);
        }
    } else {
        animState.Draw(drawPos, FacingDirection::Right, size_);
    }
}

void FireFlower::InteractWith(Character& other) {
    // FireFlower doesn't initiate interactions, Player does.
}

void FireFlower::AcceptInteract(CharacterVisitor& visitor) {
    visitor.Visit(*this);
}
