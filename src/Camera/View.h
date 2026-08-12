#pragma once
#include "raylib.h"

class View {
private:
    Camera2D camera;
    float logicalHeight;
    float maxTargetX;
    float mapPixelWidth;
    float mapPixelHeight;

    void ClampToBounds();

public:
    // Initialize the View with the number of blocks to fit vertically and the size of each block.
    // Also takes the total map width and height in pixels for boundary clamping.
    View(float blocksY = 16.0f, float blockSize = 16.0f);

    void Init(float mapPixelWidth, float mapPixelHeight);

    // Call this every frame with the target to track (e.g., Mario's position)
    void Update(float targetX, float targetY);

    void BeginDraw() const;
    void EndDraw() const;

    // Helper to manually draw a block (e.g., for testing or if TileMap is bypassed)
    void DrawBlock(float x, float y, float width, float height, Color color) const;

    float GetWorldLeft() const;
    float GetWorldTop() const;
    const Camera2D& GetRawCamera() const;
};
