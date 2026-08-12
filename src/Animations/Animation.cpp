#include "Animation.h"
#include "TextureManager/TextureManager.h"
#include <cmath>

// Helper to draw
void DrawAnim(const std::string& key, int frame, const Vector2& position, FacingDirection facing, const Vector2& scale) {
    if (!TextureManager::Has(key)) return;
    
    // Hardcoded sizes for now (16x30) per the plan, but could be dynamic
    int tileW = 16;
    int tileH = 30;
    
    Rectangle srcRect = TextureManager::GetSourceRect(key, tileW, tileH, frame);
    
    // Flip horizontal if facing left
    if (facing == FacingDirection::Left) {
        srcRect.width = -srcRect.width;
    }
    
    Rectangle destRect = { position.x, position.y, scale.x, scale.y };
    DrawTexturePro(TextureManager::Get(key), srcRect, destRect, {0, 0}, 0.0f, WHITE);
}


// --- PoseAnimation ---
void PoseAnimation::Update(float dt) {
    animTimer += dt;
    if (currentFrame == 0) {
        if (animTimer >= 3.0f) { // Stay at frame 0 for a long time (e.g. 3 seconds)
            currentFrame = 1;
            animTimer = 0.0f;
        }
    } else {
        if (animTimer >= 0.8f) { // Stay at alternate frame for a short time (e.g. 0.8 seconds)
            currentFrame = 0;
            animTimer = 0.0f;
        }
    }
}

void PoseAnimation::Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) {
    int drawFrame = 0;
    if (currentFrame != 0) {
        drawFrame = (facing == FacingDirection::Right) ? 1 : 2;
    }
    DrawAnim("mario_pose", drawFrame, position, facing, scale);
}

// --- WalkAnimation ---
void WalkAnimation::Update(float dt) {
    animTimer += dt;
    if (animTimer >= 0.1f) { // 10 FPS
        animTimer = 0.0f;
        currentFrame = (currentFrame + 1) % 3; // 3 walk frames (based on 32x60 spritesheet)
    }
}

void WalkAnimation::Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) {
    DrawAnim("mario_walk", currentFrame, position, facing, scale);
}

// --- JumpAnimation ---
void JumpAnimation::Update(float dt) {
    // Jump usually has a single frame or holds the frame
}

void JumpAnimation::Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) {
    DrawAnim("mario_jump", 0, position, facing, scale);
}

// --- SlideAnimation ---
void SlideAnimation::Update(float dt) {
    // Slide usually has a single frame
}

void SlideAnimation::Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) {
    if(facing == FacingDirection::Left) facing = FacingDirection::Right;
    else facing = FacingDirection::Left;
    DrawAnim("mario_slide", 0, position, facing, scale);
}
