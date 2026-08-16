#include "Animation.h"
#include "TextureManager/TextureManager.h"
#include <cmath>

void AnimationState::Update(float dt) {
    if (!currentAnim || currentAnim->frameCount <= 1) return;

    animTimer += dt;
    float currentDuration = currentAnim->GetDuration(currentFrameOffset);

    if (animTimer >= currentDuration) {
        animTimer = 0.0f; // Reset instead of subtract to prevent spiral if dt is large, but usually doesn't matter
        currentFrameOffset = (currentFrameOffset + 1) % currentAnim->frameCount;
    }
}

void AnimationState::Draw(const Vector2& position, FacingDirection facing, const Vector2& scale) {
    if (!currentAnim) return;

    if (!TextureManager::Has(currentAnim->textureKey)) return;
    
    int tileW = currentAnim->tileW;
    int tileH = currentAnim->tileH;
    int frameIndex = currentAnim->startFrame + currentFrameOffset;
    
    Rectangle srcRect = TextureManager::GetSourceRect(currentAnim->textureKey, tileW, tileH, frameIndex);
    
    // Flip horizontal if facing left
    if (facing == FacingDirection::Left) {
        srcRect.width = -srcRect.width;
    }
    
    Vector2 drawPos = { position.x, position.y };
    Rectangle destRect = { drawPos.x, drawPos.y, scale.x, scale.y };
    DrawTexturePro(TextureManager::Get(currentAnim->textureKey), srcRect, destRect, {0, 0}, 0.0f, WHITE);
}
