#include "DragonBoss.h"
#include "DragonBossState.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
#include "physics/CollisionSystem.h"
#include "World/BlockGrid.h"

namespace {
static Animation s_bossIdleAnim("boss_idle", 46, 40, 0, 1, {0.2f});
static Animation s_bossWalkAnim("boss_walk", 56, 40, 0, 4, {0.28f, 0.28f, 0.28f, 0.28f});
static Animation s_bossJumpWindupAnim("boss_jump", 40, 40, 0, 1, {0.35f});
static Animation s_bossJumpAirAnim("boss_jump", 40, 40, 1, 1, {0.2f});
static Animation s_bossFireAnim("boss_doFire", 55, 42, 0, 7, {0.18f, 0.18f, 0.18f, 0.18f, 0.45f, 0.18f, 0.80f});
static Animation s_bossScreamAnim("boss_scream", 50, 42, 0, 4, {0.38f, 0.38f, 0.42f, 0.42f});
static Animation s_bossIntroRoarAnim("boss_scream", 50, 42, 0, 2, {0.30f, 0.30f});
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
    SetState(new DragonIntroState());
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

void DragonBoss::UpdateFlameStream(Vector2 mouthPos, float dir, float growth) {
    hasFlameStreamUpdate_ = true;
    flameStreamMouth_ = mouthPos;
    flameStreamDir_ = dir;
    flameStreamGrowth_ = growth;
    flameStreamEnded_ = false;
}

void DragonBoss::EndFlameStream() {
    hasFlameStreamUpdate_ = true;
    flameStreamEnded_ = true;
}

bool DragonBoss::GetFlameStreamUpdate(Vector2& outMouthPos, float& outDir, float& outGrowth, bool& outEnded) {
    if (!hasFlameStreamUpdate_) return false;
    hasFlameStreamUpdate_ = false;
    outMouthPos = flameStreamMouth_;
    outDir = flameStreamDir_;
    outGrowth = flameStreamGrowth_;
    outEnded = flameStreamEnded_;
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

float DragonBoss::GetFloorYUnderFeet() const {
    if (!collisionGrid_) return -1.0f;
    int tileSize = collisionGrid_->GetTileSize();
    if (tileSize <= 0) return -1.0f;

    int leftCol = (int)((position_.x + 20.0f) / tileSize);
    int rightCol = (int)((position_.x + size_.x - 20.0f) / tileSize);
    leftCol = std::max(0, leftCol);
    rightCol = std::min(collisionGrid_->GetWidth() - 1, rightCol);

    // Look for true walkable ground (ignore ceiling rows r <= 2, ensure empty air above)
    for (int r = 3; r < collisionGrid_->GetHeight(); ++r) {
        for (int c = leftCol; c <= rightCol; ++c) {
            if (collisionGrid_->IsSolidAt(c, r) || collisionGrid_->GetBlock(c, r) != nullptr) {
                if (r > 0 && !collisionGrid_->IsSolidAt(c, r - 1) && collisionGrid_->GetBlock(c, r - 1) == nullptr) {
                    return (float)(r * tileSize);
                }
            }
        }
    }

    // Fallback: lowest solid row
    for (int r = collisionGrid_->GetHeight() - 1; r >= 3; --r) {
        for (int c = leftCol; c <= rightCol; ++c) {
            if (collisionGrid_->IsSolidAt(c, r) || collisionGrid_->GetBlock(c, r) != nullptr) {
                return (float)(r * tileSize);
            }
        }
    }
    return -1.0f;
}

void DragonBoss::Update(float dt) {
    if (invulnTimer_ > 0.0f) {
        invulnTimer_ -= dt;
        if (invulnTimer_ < 0.0f) invulnTimer_ = 0.0f;
    }

    // Apply standard physics gravity
    bool inAirborneJump = false;
    if (dynamic_cast<DragonJumpState*>(state_)) {
        inAirborneJump = true;
    }

    if (!inAirborneJump) {
        physicsBody_.velocity.y += 1800.0f * dt;
        if (physicsBody_.velocity.y > 1200.0f) physicsBody_.velocity.y = 1200.0f;
        position_.y += physicsBody_.velocity.y * dt;

        float floor = GetFloorYUnderFeet();
        if (floor > 0.0f && position_.y + size_.y >= floor) {
            position_.y = floor - size_.y;
            physicsBody_.velocity.y = 0.0f;
            physicsBody_.isGrounded = true;
            groundY_ = position_.y;
        }
        SyncPhysicsBody();
    }

    if (state_) state_->UpdateState(*this, dt);
    animState.Update(dt);
}

void DragonBoss::OnEnrageTriggered() {
    pendingEnrageSignal_ = true;
}

Rectangle DragonBoss::GetRect() const {
    float baseH = 40.0f * Global::GAME_SCALE;
    float scale = (baseH > 0.0f) ? (size_.y / baseH) : 1.0f;

    // Normal insets at scale 1.0x
    float frontInset = 6.0f * Global::GAME_SCALE;
    float backInset = 2.0f * Global::GAME_SCALE; // Wider hitbox towards the back
    float topInset = 0.0f;

    // When scaled up (e.g. 1.0x -> 2.0x during fire breath):
    // Prevent hitbox from ballooning out too far
    if (scale > 1.05f) {
        float extra = scale - 1.0f; // 0.0 to 1.0
        frontInset += extra * 18.0f * Global::GAME_SCALE;
        backInset += extra * 8.0f * Global::GAME_SCALE;
        topInset = extra * 10.0f * Global::GAME_SCALE;
    }

    if (facing_ == FacingDirection::Left) {
        return Rectangle{ position_.x + frontInset, position_.y + topInset, size_.x - (frontInset + backInset), size_.y - topInset };
    } else {
        return Rectangle{ position_.x + backInset, position_.y + topInset, size_.x - (frontInset + backInset), size_.y - topInset };
    }
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

void DragonBoss::PlayIntroRoarAnim() {
    animState.SetAnimation(&s_bossIntroRoarAnim);
    size_ = { 50.0f * Global::GAME_SCALE, 42.0f * Global::GAME_SCALE };
}

void DragonBoss::DrawBoss() {
    if (animState.GetAnimation()) {
        Vector2 drawPos = { position_.x, position_.y };
        FacingDirection renderFacing = (facing_ == FacingDirection::Left) ? FacingDirection::Right : FacingDirection::Left;
        animState.Draw(drawPos, renderFacing, size_);
    } else {
        DrawRectangle((int)position_.x, (int)position_.y, (int)size_.x, (int)size_.y, MAROON);
    }

    // Render Hitbox for calibration (Semi-transparent Green + Red Outline)
    Rectangle hitRec = GetRect();
    DrawRectangleRec(hitRec, Color{ 0, 255, 0, 90 });
    DrawRectangleLinesEx(hitRec, 2.0f, RED);

    if (auto* spawning = dynamic_cast<SpawnState*>(GetState())) {
        if (spawning->ShowWarningLine()) {
            DrawLine((int)(position_.x + size_.x / 2.0f), 0,
                     (int)(position_.x + size_.x / 2.0f), (int)GetPosition().y + 400, Fade(RED, 0.7f));
        }
    }
}