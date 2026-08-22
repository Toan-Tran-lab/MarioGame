#include "Princess.h"
#include "TextureManager/TextureManager.h"

static const Animation princessIdleAnim("princess_idle", 16, 32, 0, 2, {0.5f});

Princess::Princess() {
    animState.SetAnimation(&princessIdleAnim);
}

void Princess::Update(float dt) {
    animState.Update(dt);
}

void Princess::Draw() {
    if (!TextureManager::Has("princess_idle")) {
        TextureManager::Load("princess_idle", "assets/textures/Princess/idle/princess.png");
    }
    Vector2 drawPos = { position_.x, position_.y };
    animState.Draw(drawPos, FacingDirection::Right, size_);
}