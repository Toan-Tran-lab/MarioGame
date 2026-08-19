#include "Flagpole.h"
#include "Global/Global.h"
#include <algorithm>
#include <cmath>

void Flagpole::AddPoleSegment(Vector2 worldPos) {
    poleSegments_.push_back(worldPos);
}

void Flagpole::SetFlagPosition(Vector2 worldPos) {
    flagStartY_ = worldPos.y;
    flagY_ = worldPos.y;
    hasFlag_ = true;
}

void Flagpole::Finalize() {
    if (poleSegments_.empty()) return;

    tileSize_ = Global::TILE_SIZE * Global::GAME_SCALE;
    scale_ = Global::GAME_SCALE;

    // All pole segments share the same X; find the min/max Y
    poleX_ = poleSegments_[0].x;
    poleTop_ = poleSegments_[0].y;
    poleBottom_ = poleSegments_[0].y;

    for (const auto& seg : poleSegments_) {
        if (seg.y < poleTop_) poleTop_ = seg.y;
        if (seg.y > poleBottom_) poleBottom_ = seg.y;
    }
    // poleBottom_ is the top-left of the lowest segment; the actual bottom is + tileSize
    poleBottom_ += tileSize_;

    // Flag destination: bottom of pole minus one tile height (flag sits on last segment)
    flagEndY_ = poleBottom_ - tileSize_;

    // If no explicit flag was set, default to top of pole
    if (!hasFlag_) {
        flagStartY_ = poleTop_;
        flagY_ = poleTop_;
        hasFlag_ = true;
    }

    initialized_ = true;
}

void Flagpole::InsertSolidBlocks(BlockGrid& /*grid*/, int /*tileSize*/) {
    // The flagpole is NOT solid — Mario walks into it to trigger the flag.
    // This method is kept for interface compatibility but does nothing.
}

Rectangle Flagpole::GetTriggerBounds() const {
    if (!initialized_) return { 0, 0, 0, 0 };

    // Trigger zone: the full pole column.
    // Mario approaches from the left and overlaps the pole to trigger.
    return {
        poleX_,
        poleTop_,
        tileSize_,
        poleBottom_ - poleTop_
    };
}

void Flagpole::Trigger() {
    if (state_ == State::Idle) {
        state_ = State::FlagDescending;
    }
}

void Flagpole::Trigger(float marioY) {
    if (state_ == State::Idle) {
        // Clamp Mario's Y to the pole range so the flag doesn't start
        // above the top or below the bottom.
        float clampedY = std::max(poleTop_, std::min(marioY, flagEndY_));
        flagY_ = clampedY;
        flagStartY_ = clampedY;
        state_ = State::FlagDescending;
    }
}

bool Flagpole::IsComplete() const {
    return state_ == State::Complete;
}

void Flagpole::Update(float dt) {
    if (state_ == State::FlagDescending && hasFlag_) {
        // Flag (and Mario) slide down together
        flagY_ += flagSpeed_ * dt;
        if (flagY_ >= flagEndY_) {
            flagY_ = flagEndY_;
            state_ = State::Complete;
        }
    }
}

void Flagpole::Draw() {
    if (!initialized_ || !hasFlag_ || state_ == State::Idle) return;

    // Draw the flag tile at its current Y position
    // The flag texture comes from the main tileset (Tileset1 / ldtk_Terrain_1)
    const char* texKey = "ldtk_Terrain_1";
    if (!TextureManager::Has(texKey)) return;

    Texture2D& tex = TextureManager::Get(texKey);

    // Source rect with half-texel inset to avoid bleeding (matching TileMap::Draw)
    const float e = 0.5f;
    Rectangle src = {
        flagSrcRect_.x + e,
        flagSrcRect_.y + e,
        flagSrcRect_.width - 2.0f * e,
        flagSrcRect_.height - 2.0f * e
    };

    // The flag tile hangs to the left of the pole.
    // In the tileset, the flag sprite faces left from the pole.
    Rectangle dest = {
        poleX_,
        flagY_,
        tileSize_,
        tileSize_
    };

    DrawTexturePro(tex, src, dest, { 0, 0 }, 0.0f, WHITE);
}
