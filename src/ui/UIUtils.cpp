#include "ui/UIUtils.h"
#include <cmath>

namespace UIUtils {

    void DrawBlinkingText(const char* text, int x, int y, int fontSize, Color color, float timeAccum) {
        float blink = sinf(timeAccum * 4.0f) * 0.3f + 0.7f;
        Color drawColor = color;
        drawColor.a = (unsigned char)(blink * 255);
        DrawText(text, x, y, fontSize, drawColor);
    }

    void DrawCenteredText(const char* text, int y, int fontSize, Color color, int screenWidth) {
        int textW = MeasureText(text, fontSize);
        DrawText(text, (screenWidth - textW) / 2, y, fontSize, color);
    }

    void DrawKeyPrompt(const char* key, const char* label, float& x, float y, int fontSize, int spacing) {
        int keyW = MeasureText(key, fontSize);
        DrawText(key, (int)x, (int)y, fontSize, YELLOW);
        DrawText(label, (int)(x + keyW + 5), (int)y, fontSize, WHITE);
        x += spacing;
    }

}
