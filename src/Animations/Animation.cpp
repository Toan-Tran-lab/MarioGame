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
    // Pose is a single frame, no update needed
}

void PoseAnimation::Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) {
    DrawAnim("mario_pose", 0, position, facing, scale);
}

// --- WalkAnimation ---
void WalkAnimation::Update(float dt) {
    animTimer += dt;
    if (animTimer >= 0.1f) { // 10 FPS
        animTimer = 0.0f;
        currentFrame = (currentFrame + 1) % 4; // 4 walk frames
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
    DrawAnim("mario_slide", 0, position, facing, scale);
}
