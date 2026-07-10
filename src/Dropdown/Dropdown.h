#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class Dropdown {
private:
    Rectangle bounds;
    std::vector<std::string> options;
    int selectedIndex;
    bool isOpen;
    bool wasChanged;
    int hoveredIndex;
    bool prevMouseDown;
    int scrollOffset;
    bool isDragging;

    Color barColor;
    Color barHoverColor;
    Color listColor;
    Color listHoverColor;
    Color selectedColor;
    Color textColor;

    static constexpr int VISIBLE_ITEMS = 3;
    static constexpr float SCROLLBAR_WIDTH_RATIO = 0.04f;

    Rectangle GetItemRect(int visibleIndex) const;
    Rectangle GetScrollbarTrack() const;
    Rectangle GetScrollbarThumb() const;
    int GetMaxScrollOffset() const;

public:
    Dropdown();
    Dropdown(Rectangle bounds, const std::vector<std::string>& options, int defaultIndex,
             Color barColor, Color barHoverColor, Color listColor,
             Color listHoverColor, Color selectedColor, Color textColor);

    void SetBounds(Rectangle newBounds);
    void Update(Vector2 mousePos, bool mouseDown, float mouseWheel);
    void Draw();

    int GetSelectedIndex() const;
    bool IsChanged();
    void SetSelectedIndex(int index);
};
