#include "Button.h"
#include "Global/Global.h"

Button::Button()
    : bounds({0, 0, 0, 0}), text(""),
      normalColor(GRAY), hoverColor(DARKGRAY), currentColor(GRAY),
      isHovered(false), wasClicked(false), prevMouseDown(true) {}

Button::Button(Rectangle bounds, const std::string& text, Color normal, Color hover)
    : bounds(bounds), text(text), normalColor(normal), hoverColor(hover),
      currentColor(normal), isHovered(false), wasClicked(false), prevMouseDown(true) {}

void Button::SetBounds(Rectangle newBounds) {
    bounds = newBounds;
}

void Button::Update(Vector2 mousePos, bool mouseDown) {
    wasClicked = false;
    bool clicked = mouseDown && !prevMouseDown;
    isHovered = CheckCollisionPointRec(mousePos, bounds);

    if (isHovered && clicked) {
        wasClicked = true;
    }

    currentColor = isHovered ? hoverColor : normalColor;
    prevMouseDown = mouseDown;
}

void Button::Draw() {
    // 1. Drop shadow
    DrawRectangleRounded({bounds.x + 4, bounds.y + 4, bounds.width, bounds.height}, 0.15f, 10, Color{10, 5, 20, 120});
    
    // 2. Main body
    DrawRectangleRounded(bounds, 0.15f, 10, currentColor);
    
    // 3. Inner 3D Bevel effect (highlight at top-left)
    DrawRectangleRoundedLinesEx({bounds.x + 1, bounds.y + 1, bounds.width - 2, bounds.height - 2}, 0.15f, 10, 1.0f, Color{255, 255, 255, 100});
    
    // 4. Outer border (thick gold glow if hovered, dark border otherwise)
    if (isHovered) {
        DrawRectangleRoundedLinesEx(bounds, 0.15f, 10, 3.0f, GOLD);
    } else {
        DrawRectangleRoundedLinesEx(bounds, 0.15f, 10, 2.0f, Color{50, 35, 20, 255});
    }

    // 5. Text with drop shadow
    int fontSize = (int)(bounds.height * 0.45f);
    int maxW = (int)bounds.width - 20;
    int textW = MeasureText(text.c_str(), fontSize);
    while (textW > maxW && fontSize > 10) {
        fontSize--;
        textW = MeasureText(text.c_str(), fontSize);
    }
    
    int tx = (int)(bounds.x + (bounds.width - textW) / 2);
    int ty = (int)(bounds.y + (bounds.height - fontSize) / 2);
    
    Color textColor = isHovered ? WHITE : Color{230, 230, 230, 255};
    
    // Draw main text (will be automatically outlined by the global macro)
    DrawText(text.c_str(), tx, ty, fontSize, textColor);
}

bool Button::IsClicked() const {
    return wasClicked;
}

Rectangle Button::GetBounds() const {
    return bounds;
}
