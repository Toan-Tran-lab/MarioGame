#include "ui/UIUtils.h"
#include "ui/UIUtils.h"
#include "SaveManager/SaveManager.h"
#include "Level/Level.h"
#include "World/TileMap.h"
#include "Global/Global.h"
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

    // --- Menu Background Logic ---
    static TileMap currentBgMap;
    static bool bgInitialized = false;
    static float bgTimer = 0.0f;
    static int currentBgLevel = 1;
    static Camera2D bgCam = {0};
    static float bgCamSpeed = 50.0f;
    static std::string cachedLevelName = "1-1";

    void LoadNextBgMap() {
        Level lvl = Level::GetLevel(currentBgLevel);
        cachedLevelName = lvl.GetDisplayName();
        std::string ldtkPath = "assets/maps/maps.ldtk";
        if (currentBgMap.LoadFromLdtk(ldtkPath, lvl.GetLdtkLevelId())) {
            int sw = GetScreenWidth();
            int sh = GetScreenHeight();
            bgCam.offset = { sw / 2.0f, sh / 2.0f };
            bgCam.rotation = 0.0f;
            bgCam.zoom = (float)sh / Global::BASE_HEIGHT;

            // Pick random start position
            float mapW = (float)currentBgMap.GetPixelWidth();
            float mapH = (float)currentBgMap.GetPixelHeight();
            
            float camY = mapH - (sh / 2.0f) / bgCam.zoom; // default focus on bottom
            
            float halfViewW = (sw / 2.0f) / bgCam.zoom;
            float minX = halfViewW;
            float maxX = mapW - halfViewW;
            if (maxX < minX) maxX = minX;
            
            float camX = minX + (maxX - minX) * (GetRandomValue(0, 100) / 100.0f);
            
            bgCam.target = { camX, camY };
            bgCamSpeed = (GetRandomValue(0, 1) == 0) ? 50.0f : -50.0f;
        }
        
        currentBgLevel++;
        if (currentBgLevel > Level::GetTotalLevels()) {
            currentBgLevel = 1;
        }
    }

    void InitMenuBackground() {
        if (bgInitialized) return;
        currentBgLevel = 1;
        bgTimer = 0.0f;
        LoadNextBgMap();
        bgInitialized = true;
    }

    void UpdateMenuBackground(float dt) {
        if (!bgInitialized) return;
        
        bgTimer += dt;
        if (bgTimer >= 3.0f) {
            bgTimer = 0.0f;
            LoadNextBgMap();
        }
        
        // Panning logic
        bgCam.target.x += bgCamSpeed * dt;
        
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        bgCam.zoom = (float)sh / Global::BASE_HEIGHT;
        bgCam.offset = { sw / 2.0f, sh / 2.0f };
        float halfViewW = (sw / 2.0f) / bgCam.zoom;
        float mapW = (float)currentBgMap.GetPixelWidth();
        
        if (bgCam.target.x < halfViewW) {
            bgCam.target.x = halfViewW;
            bgCamSpeed = -bgCamSpeed; // Reverse
        } else if (bgCam.target.x > mapW - halfViewW) {
            bgCam.target.x = mapW - halfViewW;
            bgCamSpeed = -bgCamSpeed; // Reverse
        }
    }

    void DrawMenuBackground(float sw, float sh) {
        if (!bgInitialized) return;
        
        float halfViewW = (sw / 2.0f) / bgCam.zoom;
        float halfViewH = (sh / 2.0f) / bgCam.zoom;
        float worldLeft = bgCam.target.x - halfViewW;
        float worldTop = bgCam.target.y - halfViewH;
        
        ClearBackground(currentBgMap.GetBackgroundColor());
        
        BeginMode2D(bgCam);
        currentBgMap.Draw(worldLeft, worldTop, bgCam.zoom);
        EndMode2D();

        DrawRectangle(0, 0, (int)sw, (int)sh, Color{ 0, 0, 0, 140 });

        int fontSize = (int)(sh * 0.035f);
        DrawText("MARIO", (int)(sw * 0.05f), (int)(sh * 0.03f), fontSize, WHITE);
        DrawText("000000", (int)(sw * 0.05f), (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

        const char* worldLabel = "WORLD";
        int worldX = (int)((sw - MeasureText(worldLabel, fontSize)) * 0.5f);
        DrawText(worldLabel, worldX, (int)(sh * 0.03f), fontSize, WHITE);
        
        int levelW = MeasureText(cachedLevelName.c_str(), fontSize);
        DrawText(cachedLevelName.c_str(), (int)(worldX + MeasureText(worldLabel, fontSize)/2 - levelW/2), (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

        const char* timeLabel = "TIME";
        int timeX = (int)(sw - MeasureText(timeLabel, fontSize) - sw * 0.05f);
        DrawText(timeLabel, timeX, (int)(sh * 0.03f), fontSize, WHITE);

        DrawCenteredText("SUPER MARIO", (int)(sh * 0.22f), (int)(sh * 0.08f), YELLOW, (int)sw);
    }

    void CleanupMenuBackground() {
        bgInitialized = false;
    }

}
