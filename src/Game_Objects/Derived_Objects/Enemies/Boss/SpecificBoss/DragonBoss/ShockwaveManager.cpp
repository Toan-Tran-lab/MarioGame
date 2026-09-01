#include "ShockwaveManager.h"
#include "World/BlockGrid.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include <cmath>
#include <algorithm>

void ShockwaveManager::Trigger(Vector2 origin, float floorY, float maxRadius, float speed) {
    GroundShockwave wave;
    wave.origin = origin;
    wave.floorY = floorY;
    wave.maxRadius = maxRadius;
    wave.waveSpeed = speed;
    wave.currentRadius = 0.0f;
    wave.active = true;
    waves_.push_back(wave);
}

void ShockwaveManager::Update(float dt, BlockGrid& blockGrid, const std::vector<Player*>& players) {
    if (waves_.empty()) return;

    int tileSize = blockGrid.GetTileSize();
    if (tileSize <= 0) tileSize = 48; // fallback default tile size

    for (auto& wave : waves_) {
        if (!wave.active) continue;

        float prevRadius = wave.currentRadius;
        wave.currentRadius += wave.waveSpeed * dt;

        int minCol = std::max(0, (int)std::floor((wave.origin.x - wave.currentRadius) / tileSize));
        int maxCol = std::min(blockGrid.GetWidth() - 1, (int)std::floor((wave.origin.x + wave.currentRadius) / tileSize));

        int targetRow = (int)std::floor(wave.floorY / tileSize);

        for (int col = minCol; col <= maxCol; ++col) {
            if (wave.bumpedCols.find(col) == wave.bumpedCols.end()) {
                wave.bumpedCols.insert(col);

                // Find the ground block at or directly underneath floorY
                for (int r = targetRow - 1; r <= targetRow + 2; ++r) {
                    if (r >= 0 && r < blockGrid.GetHeight()) {
                        Block* b = blockGrid.GetBlock(col, r);
                        if (b && b->IsSolid()) {
                            b->Bump();
                            break;
                        }
                    }
                }
            }
        }

        // Damage any grounded player caught in the expanding wavefront
        for (Player* p : players) {
            if (!p || p->IsDead() || p->IsInvincible() || p->IsHitInvincible()) continue;
            if (wave.hitPlayers.find(p) != wave.hitPlayers.end()) continue;

            float pLeft = p->GetPosition().x;
            float pRight = p->GetPosition().x + p->GetSize().x;
            float pBottom = p->GetPosition().y + p->GetSize().y;

            bool isGrounded = p->GetPhysicsBody().isGrounded || std::abs(pBottom - wave.floorY) <= 24.0f;

            if (isGrounded) {
                float distLeft = std::abs(pLeft - wave.origin.x);
                float distRight = std::abs(pRight - wave.origin.x);
                float minDist = std::min(distLeft, distRight);
                float maxDist = std::max(distLeft, distRight);

                // If wavefront overlaps player horizontal bounds
                if (wave.currentRadius >= minDist && (wave.currentRadius - wave.waveSpeed * dt * 2.0f) <= maxDist) {
                    wave.hitPlayers.insert(p);
                    p->TakeDamage();
                }
            }
        }

        if (wave.currentRadius >= wave.maxRadius) {
            wave.active = false;
        }
    }

    waves_.erase(std::remove_if(waves_.begin(), waves_.end(),
        [](const GroundShockwave& w) { return !w.active; }), waves_.end());
}

void ShockwaveManager::Draw() const {
    for (const auto& wave : waves_) {
        if (!wave.active) continue;

        float alpha = 1.0f - (wave.currentRadius / wave.maxRadius);
        alpha = std::clamp(alpha, 0.0f, 1.0f);

        // Left ripple visual
        float leftX = wave.origin.x - wave.currentRadius;
        DrawCircle((int)leftX, (int)wave.floorY, 14.0f * alpha + 4.0f, Fade(ORANGE, alpha * 0.7f));
        DrawCircleLines((int)leftX, (int)wave.floorY, 18.0f * alpha + 4.0f, Fade(RED, alpha));

        // Right ripple visual
        float rightX = wave.origin.x + wave.currentRadius;
        DrawCircle((int)rightX, (int)wave.floorY, 14.0f * alpha + 4.0f, Fade(ORANGE, alpha * 0.7f));
        DrawCircleLines((int)rightX, (int)wave.floorY, 18.0f * alpha + 4.0f, Fade(RED, alpha));
    }
}

void ShockwaveManager::Clear() {
    waves_.clear();
}

bool ShockwaveManager::HasActiveWaves() const {
    return !waves_.empty();
}
