#include "Animation.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
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

void AnimationState::Draw(const Vector2& position, FacingDirection facing, const Vector2& scale, Color tint) {
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

    // Snap to screen pixel grid to eliminate jitter against the tilemap
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

    DrawTexturePro(TextureManager::Get(currentAnim->textureKey), srcRect, destRect, {0,0}, 0.0f, tint);
}

void AnimationState::DrawCropped(const Vector2& position, FacingDirection facing, const Vector2& scale, float cropFractionY, Color tint) {
    if (!currentAnim || cropFractionY <= 0.0f) return;

    if (!TextureManager::Has(currentAnim->textureKey)) return;
    
    int tileW = currentAnim->tileW;
    int tileH = currentAnim->tileH;
    int frameIndex = currentAnim->startFrame + currentFrameOffset;
    
    Rectangle srcRect = TextureManager::GetSourceRect(currentAnim->textureKey, tileW, tileH, frameIndex);
    srcRect.height *= cropFractionY;
    
    // Flip horizontal if facing left
    if (facing == FacingDirection::Left) {
        srcRect.width = -srcRect.width;
    }
    
    Vector2 drawPos = { position.x, position.y };
    Rectangle destRect = { drawPos.x, drawPos.y, scale.x, scale.y * cropFractionY };

    // Snap to screen pixel grid to eliminate jitter against the tilemap
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

    DrawTexturePro(TextureManager::Get(currentAnim->textureKey), srcRect, destRect, {0,0}, 0.0f, tint);
}
