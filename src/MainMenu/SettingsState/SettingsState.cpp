#include "SettingsState.h"
#include "Global/Global.h"

void SettingsState::Initialize() {
    resolutions = {
        {1920, 1080, "1920x1080"},
        {1600, 900,  "1600x900"},
        {1280, 960,  "1280x960"},
        {1280, 720,  "1280x720"},
        {1024, 768,  "1024x768"},
        {800,  600,  "800x600"},
    };

    int currentW = GetScreenWidth();
    int currentH = GetScreenHeight();
    int defaultIdx = 0;
    for (size_t i = 0; i < resolutions.size(); i++) {
        if (resolutions[i].width == currentW && resolutions[i].height == currentH) {
            defaultIdx = (int)i;
            break;
        }
    }

    std::vector<std::string> labels;
    for (auto& r : resolutions) labels.push_back(r.label);

    resolutionDropdown = Dropdown(
        {0, 0, 0, 0}, labels, defaultIdx,
        GRAY, DARKGRAY, LIGHTGRAY, SKYBLUE, BLUE, BLACK
    );

    backButton = Button({0, 0, 0, 0}, "Back", MAROON, RED);
}

void SettingsState::Update(float deltaTime) {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    float dropW = sw * 0.35f;
    float dropH = sh * 0.06f;
    float dropX = sw * 0.08f;
    float dropY = sh * 0.35f;

    resolutionDropdown.SetBounds({dropX, dropY, dropW, dropH});

    float btnW = sw * 0.2f;
    float btnH = sh * 0.07f;
    backButton.SetBounds({(sw - btnW) * 0.5f, sh * 0.75f, btnW, btnH});

    Vector2 mouse = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    float mouseWheel = GetMouseWheelMove();

    if (IsKeyPressed(KEY_UP)) {
        int idx = resolutionDropdown.GetSelectedIndex();
        idx = (idx - 1 + (int)resolutions.size()) % (int)resolutions.size();
        resolutionDropdown.SetSelectedIndex(idx);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        int idx = resolutionDropdown.GetSelectedIndex();
        idx = (idx + 1) % (int)resolutions.size();
        resolutionDropdown.SetSelectedIndex(idx);
    }

    resolutionDropdown.Update(mouse, mouseDown, mouseWheel);
    backButton.Update(mouse, mouseDown);

    if (resolutionDropdown.IsChanged()) {
        int idx = resolutionDropdown.GetSelectedIndex();
        SetWindowSize(resolutions[idx].width, resolutions[idx].height);
    }

    if (backButton.IsClicked()) {
        Global::gameStateManager->PopState();
    }
}

void SettingsState::Draw() {
    ClearBackground(RAYWHITE);

    int titleSize = (int)(GetScreenHeight() * 0.07f);
    int titleW = MeasureText("SETTINGS", titleSize);
    DrawText("SETTINGS", (GetScreenWidth() - titleW) / 2,
             (int)(GetScreenHeight() * 0.1f), titleSize, DARKGRAY);

    int labelSize = (int)(GetScreenHeight() * 0.035f);
    float dropX = GetScreenWidth() * 0.08f;
    float dropY = GetScreenHeight() * 0.35f;
    DrawText("Resolution:", (int)dropX, (int)(dropY - labelSize - 10), labelSize, DARKGRAY);

    resolutionDropdown.Draw();
    backButton.Draw();
}

void SettingsState::Cleanup() {}
