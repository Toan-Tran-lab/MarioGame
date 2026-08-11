#pragma once
#include "raylib.h"

class GameCamera {
private:
    Camera2D camera;

    // Giới hạn map (pixel) — dùng để clamp camera
    float mapPixelWidth;
    float mapPixelHeight;

    // Forward-only scrolling: camera chỉ cuộn sang phải, không bao giờ lùi lại
    float maxTargetX;

    // Clamp target để camera không vượt ra ngoài map
    void ClampToBounds();

public:
    GameCamera();

    // Khởi tạo camera với kích thước map
    void Init(float mapW, float mapH);

    // Cập nhật camera — theo dõi vị trí target (trung tâm player)
    // targetX, targetY: tọa độ world mà camera hướng tới
    void Update(float targetX, float targetY);

    // Bắt đầu / kết thúc vẽ trong hệ tọa độ camera
    void BeginDraw() const;
    void EndDraw() const;

    // Lấy vị trí world của góc trên-trái viewport (dùng cho culling)
    float GetWorldLeft() const;
    float GetWorldTop() const;

    // Truy cập Camera2D gốc (nếu cần)
    const Camera2D& GetRawCamera() const;

    // Setter
    void SetZoom(float zoom);
    void SetMapBounds(float mapW, float mapH);
};
