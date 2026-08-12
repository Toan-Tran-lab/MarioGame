#pragma once
#include "raylib.h"
#include <string>

namespace UIUtils {

    // Vẽ chữ nhấp nháy, thường dùng cho cursor (ví dụ ">")
    void DrawBlinkingText(const char* text, int x, int y, int fontSize, Color color, float timeAccum);

    // Vẽ chữ được căn giữa màn hình theo chiều ngang
    void DrawCenteredText(const char* text, int y, int fontSize, Color color, int screenWidth);

    // Vẽ phím tắt và nhãn kế bên (ví dụ "ENTER SELECT"), tự động cộng thêm spacing vào x
    void DrawKeyPrompt(const char* key, const char* label, float& x, float y, int fontSize, int spacing);

    // Menu Background Management
    void InitMenuBackground();
    void UpdateMenuBackground(float dt);
    void DrawMenuBackground(float sw, float sh);
    void CleanupMenuBackground();

}
