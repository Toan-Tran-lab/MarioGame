#include "View.h"

View::View(float blocksY, float blockSize) {
    logicalHeight = blocksY * blockSize;
    maxTargetX = 0.0f;
    mapPixelWidth = 0.0f;
    mapPixelHeight = 0.0f;

    camera.offset = { 0.0f, 0.0f };
    camera.target = { 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void View::Init(float mapPixelW, float mapPixelH) {
    mapPixelWidth = mapPixelW;
    mapPixelHeight = mapPixelH;

    camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.target = { camera.offset.x, camera.offset.y };
    maxTargetX = camera.offset.x;
}

void View::Update(float targetX, float targetY) {
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Scale so exactly 'logicalHeight' pixels fit across the screen vertically
    if (logicalHeight > 0.0f) {
        camera.zoom = screenH / logicalHeight;
    }

    camera.offset = { screenW / 2.0f, screenH / 2.0f };

    // Track target
    camera.target.x = targetX;
    camera.target.y = targetY;

    // Forward-only logic
    if (camera.target.x < maxTargetX) {
        camera.target.x = maxTargetX;
    }

    ClampToBounds();

    // Update max target after clamping
    maxTargetX = camera.target.x;
}

void View::ClampToBounds() {
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Calculate actual bounds based on current zoom
    float viewportWidth = screenW / camera.zoom;
    float viewportHeight = screenH / camera.zoom;

    float minX = viewportWidth / 2.0f;
    float maxX = mapPixelWidth - minX;
    float minY = viewportHeight / 2.0f;
    float maxY = mapPixelHeight - minY;

    if (maxX < minX) {
        camera.target.x = mapPixelWidth / 2.0f;
    } else {
        if (camera.target.x < minX) camera.target.x = minX;
        if (camera.target.x > maxX) camera.target.x = maxX;
    }

    if (maxY < minY) {
        camera.target.y = mapPixelHeight / 2.0f;
    } else {
        if (camera.target.y < minY) camera.target.y = minY;
        if (camera.target.y > maxY) camera.target.y = maxY;
    }
}

void View::BeginDraw() const {
    BeginMode2D(camera);
}

void View::EndDraw() const {
    EndMode2D();
}

void View::DrawBlock(float x, float y, float width, float height, Color color) const {
    // Note: This must be called between BeginDraw() and EndDraw().
    // The camera transform will automatically place this in world space.
    DrawRectangle((int)x, (int)y, (int)width, (int)height, color);
}

float View::GetWorldLeft() const {
    return camera.target.x - (camera.offset.x / camera.zoom);
}

float View::GetWorldTop() const {
    return camera.target.y - (camera.offset.y / camera.zoom);
}

const Camera2D& View::GetRawCamera() const {
    return camera;
}
