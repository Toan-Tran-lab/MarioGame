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

        for (const auto& block : blocks) {
            CollisionInfo col = GetCollisionInfo(body.GetRect(), block);
            if (col.side == CollisionSide::NONE) continue;

            // Prevent massive horizontal warps when jumping into ceilings
            // If it's a wall hit but the horizontal overlap is larger than our max horizontal frame movement, it's a ceiling hit!
            if (col.side == CollisionSide::LEFT || col.side == CollisionSide::RIGHT) {
                float maxValidX = std::abs(body.velocity.x) * (1.0f / 60.0f) + 2.0f;
                if (col.overlap > maxValidX) {
                    float bCY = body.position.y + body.size.y / 2.0f;
                    float blCY = block.y + block.height / 2.0f;
                    col.overlap = (body.size.y / 2.0f + block.height / 2.0f) - std::abs(bCY - blCY);
                    col.side = (bCY - blCY > 0) ? CollisionSide::TOP : CollisionSide::BOTTOM;
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
                if (body.velocity.y < 0) body.velocity.y = 0; 
            } 
            else if (col.side == CollisionSide::BOTTOM) {
                // Landed on floor
                body.position.y -= col.overlap;
                body.isGrounded = true;
                if (body.velocity.y > 0) body.velocity.y = 0;
            }
        }
    }

    void CollisionSystem::ResolveMapCollisions(PhysicsBody& body, const TileMap& tileMap) {
        body.isGrounded = false;
        
        Rectangle bodyRect = body.GetRect();
        
        // Find which grid cells the body overlaps
        Vector2 minTile = tileMap.WorldToTile(bodyRect.x, bodyRect.y);
        Vector2 maxTile = tileMap.WorldToTile(bodyRect.x + bodyRect.width, bodyRect.y + bodyRect.height);
        
        int startCol = (int)minTile.x;
        int endCol = (int)maxTile.x;
        int startRow = (int)minTile.y;
        int endRow = (int)maxTile.y;
        
        std::vector<Rectangle> solidBlocks;
        int tileSize = tileMap.GetTileSize();
        
        for (int row = startRow; row <= endRow; ++row) {
            for (int col = startCol; col <= endCol; ++col) {
                if (tileMap.IsSolidAt(col, row)) {
                    Vector2 tileWorld = tileMap.TileToWorld(col, row);
                    solidBlocks.push_back({ tileWorld.x, tileWorld.y, (float)tileSize, (float)tileSize });
                }
            }
        }
        
        // Resolve using the same logic as the vector<Rectangle> version
        if (!solidBlocks.empty()) {
            ResolveMapCollisions(body, solidBlocks);
        }
    }

} // namespace physics
