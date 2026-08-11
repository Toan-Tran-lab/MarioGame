#include "Button.h"

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
    DrawRectangleRec(bounds, currentColor);
    DrawRectangleLinesEx(bounds, 2, BLACK);

    int fontSize = (int)(bounds.height * 0.5f);
    int textW = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(),
        (int)(bounds.x + (bounds.width - textW) / 2),
        (int)(bounds.y + (bounds.height - fontSize) / 2),
        fontSize, BLACK);
}

bool Button::IsClicked() const {
    return wasClicked;
}

Rectangle Button::GetBounds() const {
    return bounds;
}
