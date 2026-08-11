#include "Player.h"
#include "Game Objects/Interaction Resolve/PlayerInteraction.h"
#include "PlayerState.h"

Player::~Player() {
    if (state != nullptr) delete state;
    state = nullptr;
}

void Player::SetState(PlayerState* Temp) {
    if (state) state->Exit(*this);
    delete state;
    state = Temp;
    if(state) state->Enter(*this);
}

void Player::TakeDamage() {
    if (state) state->OnHit(*this);
}

void Player::InteractWith(Character& other) {
    PlayerInteraction visitor(*this);
    other.AcceptInteract(visitor);
}

void Player::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void Player::Update(float dt) {
    if (state) state->UpdateState(*this, dt);
    ApplyMotion(dt);
}

void Player::Draw() {}