#include "View.h"
#include "Global/Global.h"
#include <cmath>

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

    camera.offset = { floorf(GetScreenWidth() / 2.0f), floorf(GetScreenHeight() / 2.0f) };
    camera.target = { camera.offset.x, camera.offset.y };
    maxTargetX = 0.0f;
}

void View::Update(float targetX, float targetY, float zoomMultiplier) {
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Scale so exactly 'logicalHeight' pixels fit across the screen vertically
    if (logicalHeight > 0.0f) {
        camera.zoom = (screenH / logicalHeight) * zoomMultiplier;
    }

    camera.offset = { floorf(screenW / 2.0f), floorf(screenH / 2.0f) };

    // Track target
    camera.target.x = targetX;
    camera.target.y = targetY;

    // Camera follows target smoothly in both directions (forward and backward)
    ClampToBounds();

    // Snap camera target to screen pixel grid for smoother scrolling.
    camera.target.x = roundf(camera.target.x * camera.zoom) / camera.zoom;
    camera.target.y = roundf(camera.target.y * camera.zoom) / camera.zoom;
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
    Global::currentCamera = &camera;
    BeginMode2D(camera);
}

void View::EndDraw() const {
    EndMode2D();
    Global::currentCamera = nullptr;
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
