#pragma once
#include "raylib.h"
#include "core/core.h"
#include <vector>
#include <string>

class SettingsState : public IGameState {
private:
    enum class SettingId : int {
        Resolution  = 1,
        Fullscreen  = 2,
        Volume      = 4,
        KeyBindings = 6
    };

    enum class ItemType { Header, Cycle, Toggle, SliderInt, Display };

    struct SettingItem {
        std::string label;
        std::string description;
        ItemType type;

        std::vector<std::string> cycleValues;
        int cycleIndex = 0;
        int defaultCycleIndex = 0;

        bool toggleValue = false;
        bool defaultToggle = false;

        int sliderValue = 80;
        int sliderMin = 0;
        int sliderMax = 100;
        int sliderStep = 10;
        int defaultSlider = 80;

        std::string displayValue;
    };

    std::vector<SettingItem> items;
    int selectedIndex = 0;
    bool editMode = false;
    float timeAccum = 0.0f;
    bool prevMouseDown = false;
    Vector2 lastMousePos = { -1.0f, -1.0f };
    int resWidth;
    int resHeight;

    static std::vector<std::string> resolutionLabels;
    static std::vector<std::pair<int, int>> resolutionValues;

    int GetPrevItem(int current) const;
    int GetNextItem(int current) const;
    Rectangle GetItemRect(int index, float sw, float sh) const;
    Rectangle GetArrowRect(Rectangle itemRect, bool isLeft, float sw) const;
    void ChangeValue(int dir);
    void ResetCurrentItem();
    void ApplyResolution(int idx);
    void ApplyFullscreen();
    void ApplySetting(int index);

    // Helpers for Update
    void UpdateMouseHover(float sw, float sh, Vector2 mouse);
    void HandleKeyboardNavigation();
    void HandleEditModeInput();
    void HandleMouseClick(float sw, float sh, Vector2 mouse);

    // Helpers for Draw
    void DrawTitleAndHeader(float sw, float sh) const;
    void DrawSettingsList(float sw, float sh) const;
    void DrawSingleItem(int index, Rectangle r, float sw, float sh) const;
    void DrawBottomBar(float sw, float sh) const;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
