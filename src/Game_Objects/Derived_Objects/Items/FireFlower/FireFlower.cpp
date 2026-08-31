#include "FireFlower.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"

static const Animation fireflowerAnim("fireflower", 16, 16, 0, 1, {1.0f});

FireFlower::FireFlower() {
    animState.SetAnimation(&fireflowerAnim);
    size_ = {Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE};
    // No physics needed, it just sits there
    SetActive(false);
}

void FireFlower::Update(float dt) {
    if (!IsActive()) return;
    animState.Update(dt);
}

void FireFlower::Draw() {
    if (!IsActive()) return;
    
    if (!TextureManager::Has("fireflower")) {
        TextureManager::Load("fireflower", "assets/textures/FireFlower/FireFlower.png");
    }

    animState.Draw(position_, FacingDirection::Right, size_);
}

void FireFlower::InteractWith(Character& other) {
    // FireFlower doesn't initiate interactions, Player does.
}

void FireFlower::AcceptInteract(CharacterVisitor& visitor) {
    visitor.Visit(*this);
}
