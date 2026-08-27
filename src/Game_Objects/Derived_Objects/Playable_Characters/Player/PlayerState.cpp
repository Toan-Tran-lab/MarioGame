#include "PlayerState.h"
#include "Player.h"
#include "Global/Global.h"
#include "AudioManager/AudioManager.h"

// Base default: no per-frame state logic by default.
void PlayerState::UpdateState(Player& player, float dt) {}

// --- SmallState ---

void SmallState::Enter(Player& player) {
    player.SetIsSmall(true);
    // When small, texture is 16x16.
    player.SetSize({Global::MINI_PLAYER_WIDTH * Global::GAME_SCALE, Global::MINI_PLAYER_HEIGHT * Global::GAME_SCALE});
}

void SmallState::OnHit(Player& player) {
    // Already the smallest form; there is no smaller state to shrink to.
    // The player dies, which the gameplay layer reacts to (respawn/game over).
    AudioManager::PlaySFX(AudioKey::MARIO_DIE);
    player.SetDead(true);
}

void SmallState::OnPowerup(Player& player, PowerupType type) {
    switch (type) {
        case PowerupType::Mushroom:   player.SetState(new TransformingState(new SuperState(), true)); break;
        case PowerupType::FireFlower: player.SetState(new FireState()); break;
        case PowerupType::Star:       player.SetState(new StarState()); break;
    }
}

// --- SuperState ---

void SuperState::Enter(Player& player) {
    player.SetIsSmall(false);
    // When big, texture is 16x32.
    player.SetSize({Global::SUPER_PLAYER_WIDTH * Global::GAME_SCALE, Global::SUPER_PLAYER_HEIGHT * Global::GAME_SCALE});
}

void SuperState::OnHit(Player& player) {
    AudioManager::PlaySFX(AudioKey::POWER_DOWN);
    player.SetState(new TransformingState(new SmallState(), false));
}

void SuperState::OnPowerup(Player& player, PowerupType type) {
    switch (type) {
        case PowerupType::FireFlower: player.SetState(new FireState()); break;
        case PowerupType::Star:       player.SetState(new StarState()); break;
    }
}

// --- FireState ---

void FireState::OnHit(Player& player) {
    // Classic Mario rule: Fire -> Small directly, skips Super.
    AudioManager::PlaySFX(AudioKey::POWER_DOWN);
    player.SetState(new SmallState());
}

void FireState::OnPowerup(Player& player, PowerupType type) {
    switch (type) {
        case PowerupType::Star:       player.SetState(new StarState()); break;
    }
}

// --- StarState ---

void StarState::Enter(Player& player) {
    // Hook: start the invincibility timer here.
    (void)player;
}

void StarState::OnHit(Player& player) {
    // Invincible while star-powered: damage has no effect.
    (void)player;
}

// --- TransformingState ---

TransformingState::TransformingState(PlayerState* nextState, bool isGrowing)
    : nextState_(nextState), timer_(0.0f), blinkCount_(0), isGrowing_(isGrowing), currentIsSmall_(!isGrowing) {
}

TransformingState::~TransformingState() {
    // Note: If player dies during transform, nextState_ will be deleted if we don't leak it.
    // However, SetState deletes the old state, so if we haven't transitioned, we must delete nextState_.
    if (nextState_) {
        delete nextState_;
        nextState_ = nullptr;
    }
}

void TransformingState::Enter(Player& player) {
    // Start with the opposite state size
    currentIsSmall_ = !isGrowing_;
    player.SetIsSmall(currentIsSmall_);
    
    float diffY = (Global::SUPER_PLAYER_HEIGHT - Global::MINI_PLAYER_HEIGHT) * Global::GAME_SCALE;
    
    if (currentIsSmall_) {
        player.SetSize({Global::MINI_PLAYER_WIDTH * Global::GAME_SCALE, Global::MINI_PLAYER_HEIGHT * Global::GAME_SCALE});
        // Went from Super to Small: shift down
        player.SetPosition({player.GetPosition().x, player.GetPosition().y + diffY});
    } else {
        player.SetSize({Global::SUPER_PLAYER_WIDTH * Global::GAME_SCALE, Global::SUPER_PLAYER_HEIGHT * Global::GAME_SCALE});
        // Went from Small to Super: shift up
        player.SetPosition({player.GetPosition().x, player.GetPosition().y - diffY});
    }
    
    // Small jump for effect when hit/growing (optional, but Mario typically halts)
    player.GetPhysicsBody().velocity.x = 0;
}

void TransformingState::UpdateState(Player& player, float dt) {
    // Pause physics for a classic Mario effect? (Optional, skipping for now to keep it simple)
    // The user requested: "nhấp nháy chuyển giữa 2 dạng 2 lần rồi mới biến hình trong thời gian đó vẫn chịu sát thương như thường"
    
    timer_ += dt;
    if (timer_ >= 0.15f) {
        timer_ = 0.0f;
        blinkCount_++;
        currentIsSmall_ = !currentIsSmall_;
        
        player.SetIsSmall(currentIsSmall_);
        float diffY = (Global::SUPER_PLAYER_HEIGHT - Global::MINI_PLAYER_HEIGHT) * Global::GAME_SCALE;
        
        if (currentIsSmall_) {
            player.SetSize({Global::MINI_PLAYER_WIDTH * Global::GAME_SCALE, Global::MINI_PLAYER_HEIGHT * Global::GAME_SCALE});
            player.SetPosition({player.GetPosition().x, player.GetPosition().y + diffY});
        } else {
            player.SetSize({Global::SUPER_PLAYER_WIDTH * Global::GAME_SCALE, Global::SUPER_PLAYER_HEIGHT * Global::GAME_SCALE});
            player.SetPosition({player.GetPosition().x, player.GetPosition().y - diffY});
        }
    }
    
    if (blinkCount_ >= 4) { // 4 switches = 2 blinks
        // Ensure final size matches the next state
        // If we are growing, we want to end up Super (currentIsSmall_ == false)
        // If we are shrinking, we want to end up Small (currentIsSmall_ == true)
        // At 4 blinks, it should naturally revert to the target form (opposite of what we started with)
        PlayerState* next = nextState_;
        nextState_ = nullptr; // Take ownership so it doesn't get deleted
        player.SetState(next); //Last command. DO NOT ATTEMP ANY ACTION IN THIS FUNCTION AFTER
    }
}

void TransformingState::OnHit(Player& player) {
    // If transforming from Super to Mini, player is invincible
    if (!isGrowing_) {
        return;
    }

    // "đang biến hình thì sẽ tính là đã lớn rồi nên chỉ quay lại dạng mini"
    // Whether growing or shrinking, if hit, abort transformation and go to small state
    if (nextState_) {
        delete nextState_;
        nextState_ = nullptr;
    }
    
    if (!currentIsSmall_) {
        float diffY = (Global::SUPER_PLAYER_HEIGHT - Global::MINI_PLAYER_HEIGHT) * Global::GAME_SCALE;
        player.SetPosition({player.GetPosition().x, player.GetPosition().y + diffY});
    }
    
    player.SetState(new SmallState());
}
