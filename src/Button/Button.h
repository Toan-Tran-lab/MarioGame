#pragma once
#include "raylib.h"
#include <string>

class Button {
private:
    Rectangle bounds;
    std::string text;
    Color normalColor;
    Color hoverColor;
    Color currentColor;
    bool isHovered;
    bool wasClicked;
    bool prevMouseDown;

public:
    Button();
    Button(Rectangle bounds, const std::string& text, Color normal, Color hover);
    void SetBounds(Rectangle newBounds);
    void Update(Vector2 mousePos, bool mouseDown);
    void Draw();
    bool IsClicked() const;
    Rectangle GetBounds() const;
};
