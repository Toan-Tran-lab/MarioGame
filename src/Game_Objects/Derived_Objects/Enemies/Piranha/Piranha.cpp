#include "Piranha.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
#include <cmath>
#include <algorithm>

namespace {
constexpr float kChompFrameDuration = 0.2f;
}

// 16x25 frame size, 2 frames in the 32x25 sprite sheet
static const Animation piranhaChompAnim("piranha", 16, 25, 0, 2, {kChompFrameDuration});

Piranha::Piranha() {
    animState.SetAnimation(&piranhaChompAnim);
    facing_ = FacingDirection::Right;
    size_ = { 16.0f * Global::GAME_SCALE, 25.0f * Global::GAME_SCALE };
    physicsBody_.size = size_;
}

void Piranha::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void Piranha::Update(float dt) {
    if (!spawnRecorded_) {
        baseSpawnPos_ = position_;
        // Spawn immediately at the fully emerged position (unretracted / exposed)
        position_.y = baseSpawnPos_.y;
        state_ = PiranhaState::Exposed;
        timer_ = 0.0f;
        spawnRecorded_ = true;
    }

    const float fullyEmergedY = baseSpawnPos_.y;
    const float fullyHiddenY = baseSpawnPos_.y + size_.y;

    switch (state_) {
        case PiranhaState::Exposed: {
            position_.y = fullyEmergedY;
            timer_ += dt;
            if (timer_ >= kExposedDuration) {
                state_ = PiranhaState::Retracting;
                timer_ = 0.0f;
            }
            animState.Update(dt);
            break;
        }

        case PiranhaState::Retracting: {
            position_.y += kMoveSpeed * dt;
            if (position_.y >= fullyHiddenY) {
                position_.y = fullyHiddenY;
                state_ = PiranhaState::Hidden;
                timer_ = 0.0f;
            }
            animState.Update(dt);
            break;
        }

        case PiranhaState::Hidden: {
            position_.y = fullyHiddenY;
            timer_ += dt;
            // Only start rising if hidden time elapsed and player isn't standing right on top of pipe
            bool playerDirectlyAbove = false;
            if (playerBody_) {
                float dx = std::abs((playerBody_->position.x + playerBody_->size.x / 2.0f) -
                                    (position_.x + size_.x / 2.0f));
                if (dx < size_.x * 1.5f) {
                    playerDirectlyAbove = true;
                }
            }

            if (timer_ >= kHiddenDuration && !playerDirectlyAbove) {
                state_ = PiranhaState::Rising;
                timer_ = 0.0f;
            }
            break;
        }

        case PiranhaState::Rising: {
            position_.y -= kMoveSpeed * dt;
            if (position_.y <= fullyEmergedY) {
                position_.y = fullyEmergedY;
                state_ = PiranhaState::Exposed;
                timer_ = 0.0f;
            }
            animState.Update(dt);
            break;
        }
    }

    // Sync physics body and update height to only cover the visible portion above the pipe rim
    float pipeRimY = baseSpawnPos_.y + size_.y;
    float visibleHeight = std::max(0.0f, std::min(size_.y, pipeRimY - position_.y));
    physicsBody_.position = position_;
    physicsBody_.size = { size_.x, visibleHeight };
}

void Piranha::Draw() {
    if (!TextureManager::Has("piranha")) {
        TextureManager::Load("piranha", "assets/textures/Piranha/enemies.png");
    }

    if (state_ == PiranhaState::Hidden) return;

    // Calculate the portion of the plant sticking out above the pipe rim
    float pipeRimY = baseSpawnPos_.y + size_.y;
    float visibleHeight = std::max(0.0f, std::min(size_.y, pipeRimY - position_.y));
    if (visibleHeight <= 0.0f) return;

    float visibleFraction = visibleHeight / size_.y;
    float srcH = 25.0f * visibleFraction; // Height in sprite pixels (0..25)

    int frameIndex = animState.GetCurrentFrameIndex();
    Rectangle srcRect = TextureManager::GetSourceRect("piranha", 16, 25, frameIndex);
    srcRect.height = srcH; // Clip the bottom of the sprite so stem inside pipe is hidden

    Vector2 drawPos = { position_.x, position_.y };
    Rectangle destRect = { drawPos.x, drawPos.y, size_.x, visibleHeight };

    // Snap to screen pixel grid if camera is active
    if (Global::currentCamera != nullptr) {
        float cameraZoom = Global::currentCamera->zoom;
        float cameraX = Global::currentCamera->target.x - (Global::currentCamera->offset.x / cameraZoom);
        float cameraY = Global::currentCamera->target.y - (Global::currentCamera->offset.y / cameraZoom);

        float sx0 = std::floor((destRect.x - cameraX) * cameraZoom);
        float sy0 = std::floor((destRect.y - cameraY) * cameraZoom);
        float sx1 = std::floor((destRect.x + destRect.width - cameraX) * cameraZoom);
        float sy1 = std::floor((destRect.y + destRect.height - cameraY) * cameraZoom);

        destRect.x = sx0 / cameraZoom + cameraX;
        destRect.y = sy0 / cameraZoom + cameraY;
        destRect.width = (sx1 - sx0) / cameraZoom;
        destRect.height = (sy1 - sy0) / cameraZoom;
    }

    DrawTexturePro(TextureManager::Get("piranha"), srcRect, destRect, {0, 0}, 0.0f, WHITE);
}
