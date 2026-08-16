#include "CollisionSystem.h"
#include <cmath>
#include <algorithm>

namespace physics {

    CollisionInfo CollisionSystem::GetCollisionInfo(const Rectangle& r1, const Rectangle& r2) {
        CollisionInfo info;
        
        if (!CheckCollisionRecs(r1, r2)) {
            return info;
        }

        // Calculate centers
        float r1CenterX = r1.x + r1.width / 2.0f;
        float r1CenterY = r1.y + r1.height / 2.0f;
        float r2CenterX = r2.x + r2.width / 2.0f;
        float r2CenterY = r2.y + r2.height / 2.0f;

        // Calculate differences between centers
        float dx = r1CenterX - r2CenterX;
        float dy = r1CenterY - r2CenterY;

        // Calculate minimum distances needed to not be colliding
        float minDistanceX = r1.width / 2.0f + r2.width / 2.0f;
        float minDistanceY = r1.height / 2.0f + r2.height / 2.0f;

        // Calculate penetration depths
        float overlapX = minDistanceX - std::abs(dx);
        float overlapY = minDistanceY - std::abs(dy);

        // Find the minimum translation vector (the axis of least penetration)
        if (overlapX < overlapY) {
            info.overlap = overlapX;
            info.side = (dx > 0) ? CollisionSide::LEFT : CollisionSide::RIGHT;
        } else {
            info.overlap = overlapY;
            info.side = (dy > 0) ? CollisionSide::TOP : CollisionSide::BOTTOM;
        }

        return info;
    }

    void CollisionSystem::ResolveMapCollisions(PhysicsBody& body, const std::vector<Rectangle>& blocks) {
        body.isGrounded = false; // Assume not grounded until proven otherwise
        body.hitCeiling = false; // Reset ceiling hit flag

        for (const auto& block : blocks) {
            CollisionInfo col = GetCollisionInfo(body.GetRect(), block);
            if (col.side == CollisionSide::NONE) continue;

            // Prevent massive horizontal warps when jumping into ceilings
            if (col.side == CollisionSide::LEFT || col.side == CollisionSide::RIGHT) {
                float maxValidX = std::abs(body.velocity.x) * (1.0f / 60.0f) + 2.0f;
                if (col.overlap > maxValidX) {
                    float bCY = body.position.y + body.size.y / 2.0f;
                    float blCY = block.y + block.height / 2.0f;
                    col.overlap = (body.size.y / 2.0f + block.height / 2.0f) - std::abs(bCY - blCY);
                    col.side = (bCY - blCY > 0) ? CollisionSide::TOP : CollisionSide::BOTTOM;
                } else {
                    // Prevent snagging on floor edges when walking or landing:
                    // If feet are near top of block, resolve as floor collision instead of wall collision
                    float footY = body.position.y + body.size.y;
                    if (footY >= block.y && footY <= block.y + 10.0f && body.velocity.y >= 0) {
                        col.side = CollisionSide::BOTTOM;
                        col.overlap = footY - block.y;
                    }
                }
            }
            
            // Prevent snagging on corners when moving upward (landing on floor while jumping)
            if (col.side == CollisionSide::BOTTOM && body.velocity.y < 0) {
                float bCX = body.position.x + body.size.x / 2.0f;
                float blCX = block.x + block.width / 2.0f;
                col.overlap = (body.size.x / 2.0f + block.width / 2.0f) - std::abs(bCX - blCX);
                col.side = (bCX - blCX > 0) ? CollisionSide::LEFT : CollisionSide::RIGHT;
            }

            if (col.overlap <= 0.0f) continue;

            // Resolve Collision by pushing the body out of the obstacle
            if (col.side == CollisionSide::LEFT) {
                body.position.x += col.overlap;
                if (body.velocity.x < 0) body.velocity.x = 0;
            } 
            else if (col.side == CollisionSide::RIGHT) {
                body.position.x -= col.overlap;
                if (body.velocity.x > 0) body.velocity.x = 0;
            } 
            else if (col.side == CollisionSide::TOP) {
                // Bonked head on ceiling
                body.position.y += col.overlap;
                body.velocity.y = 0; // stop upward movement
                body.hitCeiling = true;
                body.hitCeilingRect = block;
            } else if (col.side == CollisionSide::BOTTOM) {
                // Landed on floor
                body.position.y -= col.overlap;
                body.isGrounded = true;
                if (body.velocity.y > 0) body.velocity.y = 0;
            }
        }
    }

    void CollisionSystem::ResolveMapCollisions(PhysicsBody& body, const BlockGrid& blockGrid) {
        body.isGrounded = false;
        
        Rectangle bodyRect = body.GetRect();
        int tileSize = blockGrid.GetTileSize();
        if (tileSize <= 0) return;
        
        // Find which grid cells the body overlaps
        int startCol = (int)(bodyRect.x / tileSize);
        int endCol = (int)((bodyRect.x + bodyRect.width) / tileSize);
        int startRow = (int)(bodyRect.y / tileSize);
        int endRow = (int)((bodyRect.y + bodyRect.height) / tileSize);

        startRow = std::max(0, startRow);
        endRow = std::min(blockGrid.GetHeight() - 1, endRow);
        
        std::vector<Rectangle> solidBlocks;
        
        // Expand and merge adjacent solid blocks across full continuous platforms
        for (int row = startRow; row <= endRow; ++row) {
            for (int col = startCol; col <= endCol; ++col) {
                if (blockGrid.IsSolidAt(col, row)) {
                    int leftCol = col;
                    while (leftCol > 0 && blockGrid.IsSolidAt(leftCol - 1, row)) {
                        leftCol--;
                    }
                    int rightCol = col;
                    while (rightCol < blockGrid.GetWidth() - 1 && blockGrid.IsSolidAt(rightCol + 1, row)) {
                        rightCol++;
                    }

                    Rectangle mergedRect = {
                        (float)(leftCol * tileSize),
                        (float)(row * tileSize),
                        (float)((rightCol - leftCol + 1) * tileSize),
                        (float)tileSize
                    };

                    bool alreadyAdded = false;
                    for (const auto& existing : solidBlocks) {
                        if (existing.y == mergedRect.y && existing.x == mergedRect.x && existing.width == mergedRect.width) {
                            alreadyAdded = true;
                            break;
                        }
                    }

                    if (!alreadyAdded) {
                        solidBlocks.push_back(mergedRect);
                    }

                    col = rightCol;
                }
            }
        }
        
        // Resolve using expanded merged solid blocks
        if (!solidBlocks.empty()) {
            ResolveMapCollisions(body, solidBlocks);
        }
    }

} // namespace physics
