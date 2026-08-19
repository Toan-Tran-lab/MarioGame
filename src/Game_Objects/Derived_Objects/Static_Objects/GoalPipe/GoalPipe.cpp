#include "GoalPipe.h"
#include "TextureManager/TextureManager.h"

void GoalPipe::Update(float dt) {
    if (isTriggered_ && !isAnimationComplete_) {
        slideTimer_ += dt;
        playerCurrentPos_.x += slideSpeed_ * dt; // Slide RIGHT into the pipe

        if (slideTimer_ >= maxSlideDuration_) {
            isAnimationComplete_ = true;
        }
    }
}

void GoalPipe::Draw() {
    // Left empty - pipe is drawn by LDtk TileMap
}
