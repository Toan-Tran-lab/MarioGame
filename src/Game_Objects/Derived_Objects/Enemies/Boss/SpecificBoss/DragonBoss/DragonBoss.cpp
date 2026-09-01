#include "DragonBoss.h"
#include "DragonBossState.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"

namespace {
static Animation s_bossIdleAnim("boss_idle", 46, 40, 0, 1, {0.2f});
static Animation s_bossWalkAnim("boss_walk", 56, 40, 0, 4, {0.18f});
static Animation s_bossJumpWindupAnim("boss_jump", 40, 40, 0, 1, {0.35f});
static Animation s_bossJumpAirAnim("boss_jump", 40, 40, 1, 1, {0.2f});
static Animation s_bossFireAnim("boss_doFire", 55, 42, 0, 8, {0.12f});
static Animation s_bossScreamAnim("boss_scream", 50, 42, 0, 4, {0.22f});
}

DragonBoss::DragonBoss() : Boss(kMaxHp) {
    facing_ = FacingDirection::Left;
    size_ = { 56.0f * Global::GAME_SCALE, 40.0f * Global::GAME_SCALE };

    if (!TextureManager::Has("boss_idle")) {
        TextureManager::Load("boss_idle", "assets/textures/Boss/idle/boss.png");
    }
    if (!TextureManager::Has("boss_walk")) {
        TextureManager::Load("boss_walk", "assets/textures/Boss/walk/boss.png");
    }
    if (!TextureManager::Has("boss_jump")) {
        TextureManager::Load("boss_jump", "assets/textures/Boss/jump/boss.png");
    }
    if (!TextureManager::Has("boss_doFire")) {
        TextureManager::Load("boss_doFire", "assets/textures/Boss/doFire/boss.png");
    }
    if (!TextureManager::Has("boss_flame")) {
        TextureManager::Load("boss_flame", "assets/textures/Boss/flame/boss.png");
    }
    if (!TextureManager::Has("boss_scream")) {
        TextureManager::Load("boss_scream", "assets/textures/Boss/scream/boss.png");
    }

    PlayIdleAnim();
}

DragonBoss::~DragonBoss() {}

bool DragonBoss::IsEnraged() const {
    return !IsDead() && GetHp() <= GetMaxHp() / 2;
}

void DragonBoss::BeginSpawn() {
    SetState(new SpawnState());
}

int DragonBoss::HpBucket() const {
    return (GetHp() * 4) / GetMaxHp();
}

void DragonBoss::OnDamaged() {
    int bucket = HpBucket();
    if (bucket < lastHpBucket_) {
        lastHpBucket_ = bucket;
        pendingItemScatter_ = true;
    }
    if (GetHp() <= 0) {
        pendingCoinBurst_ = true;
        pendingItemScatter_ = false;
    }
    if (IsEnraged() && !enrageTriggered_ && !isDead_) {
        enrageTriggered_ = true;
        OnEnrageTriggered();
    }
}

BossState* DragonBoss::CreateIdleState() {
    return new DragonIdleState();
}

bool DragonBoss::ConsumeEnrageTriggerRequest() {
    bool v = pendingEnrageSignal_;
    pendingEnrageSignal_ = false;
    return v;
}

bool DragonBoss::ConsumeItemScatterRequest() {
    bool v = pendingItemScatter_;
    pendingItemScatter_ = false;
    return v;
}

bool DragonBoss::ConsumeCoinBurstRequest() {
    bool v = pendingCoinBurst_;
    pendingCoinBurst_ = false;
    return v;
}

void DragonBoss::RequestFlame(Vector2 origin, float direction) {
    pendingFlameSpawn_ = true;
    pendingFlameOrigin_ = origin;
    pendingFlameDir_ = direction;
}

bool DragonBoss::ConsumeFlameRequest(Vector2& outOrigin, float& outDir) {
    if (!pendingFlameSpawn_) return false;
    pendingFlameSpawn_ = false;
    outOrigin = pendingFlameOrigin_;
    outDir = pendingFlameDir_;
    return true;
}

void DragonBoss::RequestShockwave(Vector2 origin, float floorY) {
    pendingShockwave_ = true;
    pendingShockwaveOrigin_ = origin;
    pendingShockwaveFloorY_ = floorY;
}

bool DragonBoss::ConsumeShockwaveRequest(Vector2& outOrigin, float& outFloorY) {
    if (!pendingShockwave_) return false;
    pendingShockwave_ = false;
    outOrigin = pendingShockwaveOrigin_;
    outFloorY = pendingShockwaveFloorY_;
    return true;
}

void DragonBoss::UpdateAI(const std::vector<Player*>& players, float dt) {
    brain_.Update(*this, players, dt);
}

void DragonBoss::OnEnrageTriggered() {
    pendingEnrageSignal_ = true;
}

void DragonBoss::PlayIdleAnim() {
    animState.SetAnimation(&s_bossIdleAnim);
    size_ = { 46.0f * Global::GAME_SCALE, 40.0f * Global::GAME_SCALE };
}

void DragonBoss::PlayWalkAnim() {
    animState.SetAnimation(&s_bossWalkAnim);
    size_ = { 56.0f * Global::GAME_SCALE, 40.0f * Global::GAME_SCALE };
}

void DragonBoss::PlayJumpAnim() {
    animState.SetAnimation(&s_bossJumpWindupAnim);
    size_ = { 40.0f * Global::GAME_SCALE, 40.0f * Global::GAME_SCALE };
}

void DragonBoss::PlayFireAnim() {
    animState.SetAnimation(&s_bossFireAnim);
    size_ = { 55.0f * Global::GAME_SCALE, 42.0f * Global::GAME_SCALE };
}

void DragonBoss::PlayScreamAnim() {
    animState.SetAnimation(&s_bossScreamAnim);
    size_ = { 50.0f * Global::GAME_SCALE, 42.0f * Global::GAME_SCALE };
}

void DragonBoss::DrawBoss() {
    if (animState.GetAnimation()) {
        Vector2 drawPos = { position_.x, position_.y };
        animState.Draw(drawPos, facing_, size_);
    } else {
        DrawRectangle((int)position_.x, (int)position_.y, (int)size_.x, (int)size_.y, MAROON);
    }

    if (auto* spawning = dynamic_cast<SpawnState*>(GetState())) {
        if (spawning->ShowWarningLine()) {
            DrawLine((int)(position_.x + size_.x / 2.0f), 0,
                     (int)(position_.x + size_.x / 2.0f), (int)GetPosition().y + 400, Fade(RED, 0.7f));
        }
    }
}