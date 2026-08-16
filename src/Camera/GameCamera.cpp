#include "GameCamera.h"
#include "Global/Global.h"
#include <cmath>
// ========== Constructor ==========
GameCamera::GameCamera()
    : camera{0}, mapPixelWidth(0), mapPixelHeight(0), maxTargetX(0) {}

// ========== Init ==========
void GameCamera::Init(float mapW, float mapH) {
    mapPixelWidth  = mapW;
    mapPixelHeight = mapH;

    camera.zoom     = (float)GetScreenHeight() / Global::BASE_HEIGHT;
    camera.offset   = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.target   = { camera.offset.x / camera.zoom, camera.offset.y / camera.zoom };  // Bắt đầu ở góc trên-trái
    camera.rotation = 0.0f;

    // Forward-only: bắt đầu từ vị trí ban đầu
    maxTargetX = camera.target.x;
}

// ========== Update ==========
void GameCamera::Update(float targetX, float targetY) {
    // Cập nhật zoom và offset theo kích thước cửa sổ (hỗ trợ resize)
    camera.zoom   = (float)GetScreenHeight() / Global::BASE_HEIGHT;
    camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

    // Camera theo dõi target (trung tâm player) - rounded to prevent sub-pixel blur
    camera.target.x = roundf(targetX);
    camera.target.y = roundf(targetY);

    // Forward-only: camera chỉ cuộn sang phải, không bao giờ lùi lại
    // Khi player ở nửa trái màn hình → camera đứng yên
    // Khi player vượt qua giữa màn hình → camera tiến theo
    if (camera.target.x < maxTargetX) {
        camera.target.x = maxTargetX;
    }

    // Giới hạn camera trong map
    ClampToBounds();

    // Cập nhật maxTargetX sau khi clamp (tôn trọng biên phải map)
    maxTargetX = camera.target.x;
}

// ========== Clamp ==========
void GameCamera::ClampToBounds() {
    // Min/max target để viewport không vượt ra ngoài map
    float minX = camera.offset.x / camera.zoom;
    float maxX = mapPixelWidth  - (camera.offset.x / camera.zoom);
    float minY = camera.offset.y / camera.zoom;
    float maxY = mapPixelHeight - (camera.offset.y / camera.zoom);

    // Trục X
    if (maxX < minX) {
        // Map hẹp hơn màn hình → canh giữa
        camera.target.x = mapPixelWidth / 2.0f;
    } else {
        if (camera.target.x < minX) camera.target.x = minX;
        if (camera.target.x > maxX) camera.target.x = maxX;
    }

    // Trục Y
    if (maxY < minY) {
        // Map thấp hơn màn hình → canh giữa
        camera.target.y = mapPixelHeight / 2.0f;
    } else {
        if (camera.target.y < minY) camera.target.y = minY;
        if (camera.target.y > maxY) camera.target.y = maxY;
    }
}

// ========== Bắt đầu / kết thúc vẽ ==========
void GameCamera::BeginDraw() const {
    BeginMode2D(camera);
}

void GameCamera::EndDraw() const {
    EndMode2D();
}

// ========== Truy vấn ==========
float GameCamera::GetWorldLeft() const {
    return camera.target.x - (camera.offset.x / camera.zoom);
}

float GameCamera::GetWorldTop() const {
    return camera.target.y - (camera.offset.y / camera.zoom);
}

const Camera2D& GameCamera::GetRawCamera() const {
    return camera;
}

// ========== Setter ==========
void GameCamera::SetZoom(float zoom) {
    if (zoom > 0.0f) camera.zoom = zoom;
}

void GameCamera::SetMapBounds(float mapW, float mapH) {
    mapPixelWidth  = mapW;
    mapPixelHeight = mapH;
}
