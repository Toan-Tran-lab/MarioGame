#include "Dropdown.h"
#include <algorithm>

Dropdown::Dropdown()
    : bounds({0, 0, 0, 0}), selectedIndex(0), isOpen(false),
      wasChanged(false), hoveredIndex(-1), prevMouseDown(false), scrollOffset(0), isDragging(false),
      barColor(GRAY), barHoverColor(DARKGRAY), listColor(LIGHTGRAY),
      listHoverColor(SKYBLUE), selectedColor(BLUE), textColor(BLACK) {}

Dropdown::Dropdown(Rectangle bounds, const std::vector<std::string>& options, int defaultIndex,
                   Color barColor, Color barHoverColor, Color listColor,
                   Color listHoverColor, Color selectedColor, Color textColor)
    : bounds(bounds), options(options), selectedIndex(defaultIndex),
      isOpen(false), wasChanged(false), hoveredIndex(-1), prevMouseDown(false), scrollOffset(0), isDragging(false),
      barColor(barColor), barHoverColor(barHoverColor), listColor(listColor),
      listHoverColor(listHoverColor), selectedColor(selectedColor), textColor(textColor) {}

void Dropdown::SetBounds(Rectangle newBounds) {
    bounds = newBounds;
}

Rectangle Dropdown::GetItemRect(int visibleIndex) const {
    float scrollW = bounds.width * SCROLLBAR_WIDTH_RATIO;
    return {
        bounds.x,
        bounds.y + bounds.height + visibleIndex * bounds.height,
        bounds.width - scrollW - 4.0f,
        bounds.height
    };
}

Rectangle Dropdown::GetScrollbarTrack() const {
    float scrollW = bounds.width * SCROLLBAR_WIDTH_RATIO;
    return {
        bounds.x + bounds.width - scrollW,
        bounds.y + bounds.height,
        scrollW,
        VISIBLE_ITEMS * bounds.height
    };
}

Rectangle Dropdown::GetScrollbarThumb() const {
    float scrollW = bounds.width * SCROLLBAR_WIDTH_RATIO;
    float trackH = VISIBLE_ITEMS * bounds.height;
    float total = (float)options.size();
    float thumbH = trackH * (VISIBLE_ITEMS / total);
    int maxOff = GetMaxScrollOffset();
    float trackY = bounds.y + bounds.height;
    float thumbY = trackY;
    if (maxOff > 0)
        thumbY += ((float)scrollOffset / maxOff) * (trackH - thumbH);
    return {
        bounds.x + bounds.width - scrollW,
        thumbY,
        scrollW,
        thumbH
    };
}

int Dropdown::GetMaxScrollOffset() const {
    return std::max(0, (int)options.size() - VISIBLE_ITEMS);
}

void Dropdown::Update(Vector2 mousePos, bool mouseDown, float mouseWheel) {
    bool clicked = mouseDown && !prevMouseDown;
    wasChanged = false;

    bool overBar = CheckCollisionPointRec(mousePos, bounds);

    Rectangle scrollTrack = GetScrollbarTrack();
    Rectangle scrollThumb = GetScrollbarThumb();

    if (isOpen) {
        if (CheckCollisionPointRec(mousePos, scrollThumb) && clicked)
            isDragging = true;

        if (isDragging && !mouseDown)
            isDragging = false;

        if (isDragging) {
            int maxOff = GetMaxScrollOffset();
            if (maxOff > 0) {
                float trackH = VISIBLE_ITEMS * bounds.height;
                float thumbH = scrollThumb.height;
                float trackY = bounds.y + bounds.height;
                float fraction = (mousePos.y - trackY - thumbH * 0.5f) / (trackH - thumbH);
                scrollOffset = (int)(fraction * maxOff + 0.5f);
                scrollOffset = std::max(0, std::min(maxOff, scrollOffset));
            }
        }
    }

    if (overBar && mouseWheel != 0.0f && !isDragging) {
        if (!isOpen) {
            int dir = (mouseWheel > 0) ? -1 : 1;
            selectedIndex = (selectedIndex + dir + (int)options.size()) % (int)options.size();
            wasChanged = true;
        }
    }

    if (overBar && clicked && !isDragging) {
        isOpen = !isOpen;
        if (isOpen) {
            hoveredIndex = selectedIndex;
            scrollOffset = std::max(0, selectedIndex - VISIBLE_ITEMS / 2);
            scrollOffset = std::min(GetMaxScrollOffset(), scrollOffset);
        }
    }

    if (isOpen) {
        int maxOff = GetMaxScrollOffset();

        if (mouseWheel != 0.0f && !isDragging) {
            scrollOffset -= (int)mouseWheel;
            scrollOffset = std::max(0, std::min(maxOff, scrollOffset));
        }

        bool overTrack = CheckCollisionPointRec(mousePos, scrollTrack);
        bool overThumb = CheckCollisionPointRec(mousePos, scrollThumb);
        if (overTrack && !overThumb && clicked) {
            if (mousePos.y < scrollThumb.y)
                scrollOffset = std::max(0, scrollOffset - 1);
            else
                scrollOffset = std::min(maxOff, scrollOffset + 1);
        }

        hoveredIndex = -1;
        for (int i = 0; i < VISIBLE_ITEMS; i++) {
            int realIndex = scrollOffset + i;
            if (realIndex >= (int)options.size()) break;
            if (CheckCollisionPointRec(mousePos, GetItemRect(i))) {
                hoveredIndex = realIndex;
                break;
            }
        }

        if (hoveredIndex >= 0 && clicked && !isDragging) {
            selectedIndex = hoveredIndex;
            wasChanged = true;
            isOpen = false;
        }

        Rectangle listArea = {
            bounds.x,
            bounds.y + bounds.height,
            bounds.width,
            VISIBLE_ITEMS * bounds.height
        };
        bool overListArea = CheckCollisionPointRec(mousePos, listArea);
        if (clicked && !overBar && !overListArea) {
            isOpen = false;
        }
    }

    prevMouseDown = mouseDown;
    selectedIndex = std::max(0, std::min((int)options.size() - 1, selectedIndex));
}

void Dropdown::Draw() {
    bool hoverBar = CheckCollisionPointRec(GetMousePosition(), bounds);
    DrawRectangleRec(bounds, hoverBar ? barHoverColor : barColor);
    DrawRectangleLinesEx(bounds, 2, BLACK);

    if (!options.empty()) {
        int fontSize = (int)(bounds.height * 0.45f);
        const char* text = options[selectedIndex].c_str();
        int textW = MeasureText(text, fontSize);
        float tx = bounds.x + (bounds.width - textW) * 0.5f;
        float ty = bounds.y + (bounds.height - fontSize) * 0.5f;
        DrawText(text, (int)tx, (int)ty, fontSize, textColor);
    }

    if (isOpen) {
        for (int i = 0; i < VISIBLE_ITEMS; i++) {
            int realIndex = scrollOffset + i;
            if (realIndex >= (int)options.size()) break;

            Rectangle itemRect = GetItemRect(i);
            bool hover = (realIndex == hoveredIndex);
            bool selected = (realIndex == selectedIndex);
            Color bg;
            if (selected) bg = selectedColor;
            else if (hover) bg = listHoverColor;
            else bg = listColor;

            DrawRectangleRec(itemRect, bg);
            DrawRectangleLinesEx(itemRect, 1, BLACK);

            int fontSize = (int)(itemRect.height * 0.45f);
            const char* text = options[realIndex].c_str();
            int textW = MeasureText(text, fontSize);
            float tx = itemRect.x + (itemRect.width - textW) * 0.5f;
            float ty = itemRect.y + (itemRect.height - fontSize) * 0.5f;

            Color tCol = (selected && !hover) ? WHITE : textColor;
            DrawText(text, (int)tx, (int)ty, fontSize, tCol);
        }

        if ((int)options.size() > VISIBLE_ITEMS) {
            Rectangle thumb = GetScrollbarThumb();
            DrawRectangleRec(thumb, isDragging ? BLUE : DARKGRAY);
        }
    }
}

int Dropdown::GetSelectedIndex() const {
    return selectedIndex;
}

bool Dropdown::IsChanged() {
    bool ret = wasChanged;
    wasChanged = false;
    return ret;
}

void Dropdown::SetSelectedIndex(int index) {
    selectedIndex = std::max(0, std::min((int)options.size() - 1, index));
    wasChanged = true;
}
