#include "SettingsState.h"
#include "Global/Global.h"
#include "ui/UIUtils.h"
#include <cmath>
#include <algorithm>

std::vector<std::string> SettingsState::resolutionLabels;
std::vector<std::pair<int, int>> SettingsState::resolutionValues;

void SettingsState::Initialize() {
    resolutionValues = { {1920, 1080}, {1600, 900}, {1280, 960}, {1280, 720}, {1024, 768}, {800,  600} };
    resolutionLabels.clear();
    for (auto& r : resolutionValues) resolutionLabels.push_back(std::to_string(r.first) + "x" + std::to_string(r.second));

    int currentW = GetScreenWidth(), currentH = GetScreenHeight();
    int defaultResIndex = 0;
    for (size_t i = 0; i < resolutionValues.size(); i++) {
        if (resolutionValues[i].first == currentW && resolutionValues[i].second == currentH) { defaultResIndex = (int)i; break; }
    }
    resWidth = currentW; resHeight = currentH;

    items = {
        {"VIDEO", "", ItemType::Header},
        {"RESOLUTION", "Change screen resolution", ItemType::Cycle},
        {"FULLSCREEN", "Toggle fullscreen mode", ItemType::Toggle},
        {"AUDIO", "", ItemType::Header},
        {"SOUND VOLUME", "Adjust sound volume", ItemType::SliderInt},
        {"CONTROLS", "", ItemType::Header},
        {"KEY BINDINGS", "Customize controls", ItemType::Display}
    };
    items[1].cycleValues = resolutionLabels; items[1].cycleIndex = defaultResIndex; items[1].defaultCycleIndex = defaultResIndex;
    items[4].sliderValue = 80; items[4].defaultSlider = 80; items[4].sliderMin = 0; items[4].sliderMax = 100; items[4].sliderStep = 10;
    items[6].displayValue = "Coming Soon";
    selectedIndex = 1; editMode = false; prevMouseDown = false; timeAccum = 0.0f;
}

int SettingsState::GetPrevItem(int current) const {
    int count = (int)items.size();
    for (int i = 1; i < count; i++) {
        int idx = (current - i + count) % count;
        if (items[idx].type != ItemType::Header) return idx;
    }
    return current;
}

int SettingsState::GetNextItem(int current) const {
    int count = (int)items.size();
    for (int i = 1; i < count; i++) {
        int idx = (current + i) % count;
        if (items[idx].type != ItemType::Header) return idx;
    }
    return current;
}

Rectangle SettingsState::GetItemRect(int index, float sw, float sh) const {
    float itemW = sw * 0.7f, itemH = sh * 0.065f, itemX = sw * 0.12f, startY = sh * 0.18f, gap = sh * 0.015f;
    return { itemX, startY + index * (itemH + gap), itemW, itemH };
}

Rectangle SettingsState::GetArrowRect(Rectangle itemRect, bool isLeft, float sw) const {
    float arrowAreaX = itemRect.x + itemRect.width - sw * 0.3f, halfW = (sw * 0.25f) * 0.5f;
    return isLeft ? Rectangle{ arrowAreaX, itemRect.y, halfW, itemRect.height } 
                  : Rectangle{ arrowAreaX + halfW, itemRect.y, halfW, itemRect.height };
}

void SettingsState::ChangeValue(int dir) {
    SettingItem& item = items[selectedIndex];
    if (item.type == ItemType::Cycle) {
        int count = (int)item.cycleValues.size();
        item.cycleIndex = (item.cycleIndex + dir + count) % count;
        ApplySetting(selectedIndex);
    } else if (item.type == ItemType::Toggle) {
        item.toggleValue = !item.toggleValue;
        ApplySetting(selectedIndex);
    } else if (item.type == ItemType::SliderInt) {
        item.sliderValue += dir * item.sliderStep;
        item.sliderValue = std::max(item.sliderMin, std::min(item.sliderMax, item.sliderValue));
    }
}

void SettingsState::ResetCurrentItem() {
    SettingItem& item = items[selectedIndex];
    if (item.type == ItemType::Cycle) { item.cycleIndex = item.defaultCycleIndex; ApplySetting(selectedIndex); }
    else if (item.type == ItemType::Toggle) { item.toggleValue = item.defaultToggle; ApplySetting(selectedIndex); }
    else if (item.type == ItemType::SliderInt) { item.sliderValue = item.defaultSlider; }
}

void SettingsState::ApplyResolution(int idx) {
    if (idx >= 0 && idx < (int)resolutionValues.size()) {
        resWidth = resolutionValues[idx].first; resHeight = resolutionValues[idx].second;
        SetWindowSize(resWidth, resHeight);
    }
}
void SettingsState::ApplyFullscreen() { ToggleFullscreen(); }

void SettingsState::ApplySetting(int index) {
    switch (static_cast<SettingId>(index)) {
        case SettingId::Resolution: ApplyResolution(items[index].cycleIndex); break;
        case SettingId::Fullscreen: ApplyFullscreen(); break;
        default: break;
    }
}

void SettingsState::Update(float deltaTime) {
    timeAccum += deltaTime;
    UIUtils::UpdateMenuBackground(deltaTime);

    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool clicked = mouseDown && !prevMouseDown;
    prevMouseDown = mouseDown;
    
    UpdateMouseHover(sw, sh, mouse);
    
    if (editMode) HandleEditModeInput();
    else HandleKeyboardNavigation();

    if (clicked) HandleMouseClick(sw, sh, mouse);
}

void SettingsState::UpdateMouseHover(float sw, float sh, Vector2 mouse) {
    if (mouse.x == lastMousePos.x && mouse.y == lastMousePos.y) return;
    lastMousePos = mouse;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].type == ItemType::Header) continue;
        if (CheckCollisionPointRec(mouse, GetItemRect((int)i, sw, sh))) { selectedIndex = (int)i; break; }
    }
}

void SettingsState::HandleEditModeInput() {
    if (IsKeyPressed(Global::keys.select) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || 
        IsKeyPressed(Global::keys.back) || IsKeyPressed(KEY_ESCAPE)) {
        editMode = false;
    } else if (IsKeyPressed(Global::keys.reset) || IsKeyPressed(KEY_R)) {
        ResetCurrentItem();
    } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        ChangeValue(IsKeyPressed(KEY_RIGHT) ? 1 : -1);
    }
}

void SettingsState::HandleKeyboardNavigation() {
    if (IsKeyPressed(KEY_UP)) selectedIndex = GetPrevItem(selectedIndex);
    else if (IsKeyPressed(KEY_DOWN)) selectedIndex = GetNextItem(selectedIndex);
    else if (IsKeyPressed(Global::keys.select) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (items[selectedIndex].type != ItemType::Display) editMode = true;
    } else if (IsKeyPressed(Global::keys.reset) || IsKeyPressed(KEY_R)) ResetCurrentItem();
    else if (IsKeyPressed(Global::keys.back) || IsKeyPressed(KEY_ESCAPE)) Global::gameStateManager->PopState();
}

void SettingsState::HandleMouseClick(float sw, float sh, Vector2 mouse) {
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].type == ItemType::Header) continue;
        Rectangle r = GetItemRect((int)i, sw, sh);
        Rectangle leftArrow = GetArrowRect(r, true, sw), rightArrow = GetArrowRect(r, false, sw);

        if (items[i].type != ItemType::Display) {
            if (CheckCollisionPointRec(mouse, leftArrow)) {
                if (!editMode) selectedIndex = (int)i;
                if ((int)i == selectedIndex) ChangeValue(-1);
                return;
            } else if (CheckCollisionPointRec(mouse, rightArrow)) {
                if (!editMode) selectedIndex = (int)i;
                if ((int)i == selectedIndex) ChangeValue(1);
                return;
            }
        }
        if (!editMode && CheckCollisionPointRec(mouse, r)) {
            selectedIndex = (int)i;
            if (items[i].type != ItemType::Display) editMode = true;
            return;
        }
    }
    if (CheckCollisionPointRec(mouse, { sw * 0.78f, sh * 0.92f, sw * 0.2f, sh * 0.05f })) {
        if (editMode) editMode = false;
        Global::gameStateManager->PopState();
    }
}

void SettingsState::Draw() {
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    ClearBackground(Color{ 60, 40, 80, 255 });
    UIUtils::DrawMenuBackground(sw, sh);
    
    DrawTitleAndHeader(sw, sh);
    DrawSettingsList(sw, sh);
    DrawBottomBar(sw, sh);
}

void SettingsState::DrawTitleAndHeader(float sw, float sh) const {
    DrawText(items[selectedIndex].label.c_str(), (int)(sw * 0.05f), (int)(sh * 0.03f), (int)(sh * 0.035f), YELLOW);
    UIUtils::DrawCenteredText("SETTINGS", (int)(sh * 0.08f), (int)(sh * 0.06f), WHITE, (int)sw);
}

void SettingsState::DrawSettingsList(float sw, float sh) const {
    for (int i = 0; i < (int)items.size(); i++) {
        Rectangle r = GetItemRect(i, sw, sh);
        if (items[i].type == ItemType::Header) {
            DrawText(items[i].label.c_str(), (int)r.x, (int)r.y, (int)(sh * 0.035f), Color{ 180, 160, 200, 255 });
            float lineY = r.y + sh * 0.04f;
            DrawLine((int)r.x, (int)lineY, (int)(sw * 0.9f), (int)lineY, Color{ 100, 80, 120, 255 });
            continue;
        }
        DrawSingleItem(i, r, sw, sh);
    }
}

void SettingsState::DrawSingleItem(int index, Rectangle r, float sw, float sh) const {
    bool isSel = (index == selectedIndex);
    int itemFontSize = (int)(sh * 0.028f);
    
    if (isSel) {
        int iconX = (int)(r.x - sw * 0.04f);
        int iconY = (int)(r.y + (r.height - itemFontSize) * 0.5f);
        UIUtils::DrawBlinkingText(">", iconX, iconY, itemFontSize, YELLOW, timeAccum);
    }
    
    DrawText(items[index].label.c_str(), (int)r.x, (int)r.y, itemFontSize, isSel ? YELLOW : WHITE);
    if (!items[index].description.empty())
        DrawText(items[index].description.c_str(), (int)r.x, (int)(r.y + itemFontSize + 2), (int)(sh * 0.022f), Color{ 180, 180, 180, 255 });

    std::string vStr = "";
    if (items[index].type == ItemType::Cycle) vStr = items[index].cycleValues[items[index].cycleIndex];
    else if (items[index].type == ItemType::Toggle) vStr = items[index].toggleValue ? "ON" : "OFF";
    else if (items[index].type == ItemType::SliderInt) vStr = std::to_string(items[index].sliderValue) + "%";
    else if (items[index].type == ItemType::Display) vStr = items[index].displayValue;

    if (!vStr.empty()) {
        float arrowAreaX = r.x + r.width - sw * 0.3f;
        int valueY = (int)(r.y + (r.height - itemFontSize) * 0.5f);
        int valueX = (int)(arrowAreaX + (sw * 0.25f - MeasureText(vStr.c_str(), itemFontSize)) * 0.5f);

        if (items[index].type != ItemType::Display) {
            Color arrC = isSel ? YELLOW : Color{ 200, 200, 200, 255 };
            DrawText("<", (int)arrowAreaX, valueY, itemFontSize, arrC);
            DrawText(">", (int)(arrowAreaX + sw * 0.25f - MeasureText(">", itemFontSize)), valueY, itemFontSize, arrC);
        }
        DrawText(vStr.c_str(), valueX, valueY, itemFontSize, (items[index].type == ItemType::Display) ? Color{ 128, 128, 128, 255 } : WHITE);
    }
    if (editMode && isSel) DrawRectangleRec(r, Color{ 255, 255, 255, 20 });
}

void SettingsState::DrawBottomBar(float sw, float sh) const {
    int barY = (int)(sh * 0.92f);
    DrawLine(0, barY, (int)sw, barY, Color{ 100, 80, 120, 255 });
    
    int barFontSize = (int)(sh * 0.025f), spacing = (int)(sw * 0.08f);
    float x = sw - spacing * 3;
    
    UIUtils::DrawKeyPrompt("ENTER", "SELECT", x, barY + 5, barFontSize, spacing);
    UIUtils::DrawKeyPrompt("R", "RESET", x, barY + 5, barFontSize, spacing);
    UIUtils::DrawKeyPrompt("ESC", "BACK", x, barY + 5, barFontSize, spacing);
}

void SettingsState::Cleanup() { items.clear(); resolutionLabels.clear(); resolutionValues.clear(); }
