#include "PlayerState.h"

void PlayerState::Enter(Player& player) {}
void PlayerState::Exit(Player& player) {}
void PlayerState::UpdateState(Player& player, float dt) {}

void SmallState::OnHit(Player& player) override;
void SmallState::OnPowerup(Player& player, PowerupType type) override;

void SuperState::Enter(Player& player) override;
void SuperState::OnHit(Player& player) override;
void SuperState::OnPowerup(Player& player, PowerupType type) override;

void FireState::OnHit(Player& player) override;
void FireState::OnPowerup(Player& player, PowerupType type) override {}