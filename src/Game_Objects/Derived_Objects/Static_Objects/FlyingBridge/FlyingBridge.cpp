#include "FlyingBridge.h"
#include "World/BlockGrid.h"

void FlyingBridge::Update(float dt) {
    // Move horizontally
    position_.x += velocityX_ * dt;

    // Check map border collisions
    if (position_.x <= leftBound_) {
        position_.x = leftBound_;
        velocityX_ = -velocityX_;
    }
    if (position_.x + size_.x >= rightBound_) {
        position_.x = rightBound_ - size_.x;
        velocityX_ = -velocityX_;
    }

    // Check BlockGrid for solid wall ahead
    if (blockGrid_) {
        int tileSize = blockGrid_->GetTileSize();
        if (tileSize > 0) {
            // Check the entire height of the bridge
            int topRow = (int)(position_.y / tileSize);
            int bottomRow = (int)((position_.y + size_.y - 0.1f) / tileSize);
            
            topRow = std::max(0, topRow);
            bottomRow = std::min(blockGrid_->GetHeight() - 1, bottomRow);

            bool hit = false;
            if (velocityX_ > 0.0f) {
                // Moving right: check the right edge
                int col = (int)((position_.x + size_.x) / tileSize);
                if (col < blockGrid_->GetWidth()) {
                    for (int r = topRow; r <= bottomRow; ++r) {
                        if (blockGrid_->IsSolidAt(col, r)) {
                            hit = true; break;
                        }
                    }
                }
                if (hit) {
                    position_.x = (float)(col * tileSize) - size_.x;
                    velocityX_ = -velocityX_;
                }
            } else {
                // Moving left: check the left edge
                int col = (int)((position_.x - 1.0f) / tileSize);
                if (col >= 0) {
                    for (int r = topRow; r <= bottomRow; ++r) {
                        if (blockGrid_->IsSolidAt(col, r)) {
                            hit = true; break;
                        }
                    }
                }
                if (hit) {
                    position_.x = (float)((col + 1) * tileSize);
                    velocityX_ = -velocityX_;
                }
            }
        }
    }
}

#include "TextureManager/TextureManager.h"

void FlyingBridge::Draw() {
    if (!TextureManager::Has("flyingbridge")) {
        TextureManager::Load("flyingbridge", "assets/textures/Tileset/flyingbridge.png");
        return; // wait for next frame
    }

    Rectangle rect = GetRect();
    Texture2D tex = TextureManager::Get("flyingbridge");
    Rectangle src = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    DrawTexturePro(tex, src, rect, { 0.0f, 0.0f }, 0.0f, WHITE);
}
