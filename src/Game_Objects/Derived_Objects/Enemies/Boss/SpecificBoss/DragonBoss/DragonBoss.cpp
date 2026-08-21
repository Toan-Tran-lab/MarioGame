#include "DragonBoss.h"
#include "DragonBossState.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "TextureManager/TextureManager.h"

DragonBoss::DragonBoss() : Boss(kMaxHp) {
    facing_ = FacingDirection::Left;
    // NOTE: no SetState() here — this constructor runs before SetPosition()
    // is called by the spawner. Call BeginSpawn() explicitly after
    // positioning the boss, or it will sit in no state at all.
}

DragonBoss::~DragonBoss() {}

bool DragonBoss::IsEnraged() const {
    return !IsDead() && GetHp() <= GetMaxHp() / 2;
}

DragonAttackType DragonBoss::NextAttack() {
    DragonAttackType type = (attackCycleIndex_ % 3 == 2)
        ? DragonAttackType::Flamethrower
        : DragonAttackType::Stomp;
    attackCycleIndex_++;
    return type;
}

void DragonBoss::BeginSpawn() {
    SetState(new SpawnState());
}

int DragonBoss::HpBucket() const {
    return (GetHp() * 4) / GetMaxHp(); // 4=76-100%, 3=51-75%, 2=26-50%, 1=1-25%, 0=dead
}

void DragonBoss::OnDamaged() {
    int bucket = HpBucket();
    if (bucket < lastHpBucket_) {
        lastHpBucket_ = bucket;
        pendingItemScatter_ = true;
    }
    if (GetHp() <= 0) {
        pendingCoinBurst_ = true;
        pendingItemScatter_ = false; // death takes priority over a simultaneous quarter-crossing
    }
    if (IsEnraged() && !enrageTriggered_ && !isDead_) {
        enrageTriggered_ = true;
        OnEnrageTriggered();
    }
}

void DragonBoss::OnSpawnComplete() {
    SetStompBouncePosition({ GetPosition().x - 60.0f, GetPosition().y + GetSize().y - 40.0f });
}

BossState* DragonBoss::CreateIdleState() {
    return new IdleState();
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

void DragonBoss::RequestFireball(Vector2 origin) {
    pendingFireballSpawn_ = true;
    pendingFireballOrigin_ = origin;
}

bool DragonBoss::ConsumeFireballRequest(Vector2& outOrigin) {
    if (!pendingFireballSpawn_) return false;
    pendingFireballSpawn_ = false;
    outOrigin = pendingFireballOrigin_;
    return true;
}

void DragonBoss::DrawBoss() {
    if (animState.GetAnimation()) {
        Vector2 drawPos = { position_.x, position_.y };
        animState.Draw(drawPos, facing_, size_);
    } else {
        DrawRectangle((int)position_.x, (int)position_.y, (int)size_.x, (int)size_.y, MAROON);
    }

    if (auto* aiming = dynamic_cast<AimingStompState*>(GetState())) {
        Vector2 t = aiming->GetTargetPos();
        DrawCircleLines((int)t.x, (int)t.y, 24.0f, Fade(RED, 0.6f));
    }

    if (auto* spawning = dynamic_cast<SpawnState*>(GetState())) {
        if (spawning->ShowWarningLine()) {
            DrawLine((int)(position_.x + size_.x / 2.0f), 0,
                     (int)(position_.x + size_.x / 2.0f), (int)GetPosition().y + 400, Fade(RED, 0.7f));
        }
    }

    if (auto* slam = dynamic_cast<ArmSlamState*>(GetState())) { 
        if (slam->IsBursting()) {
            Vector2 t = slam->GetTargetPos();
            float progress = slam->GetBurstProgress();
            float radius = 40.0f * progress; // matches ArmSlamState::kBurstRadius
            DrawCircle((int)t.x, (int)t.y, radius, Fade(ORANGE, 1.0f - progress * 0.5f));
            DrawCircleLines((int)t.x, (int)t.y, radius, RED);
        }
    }
}