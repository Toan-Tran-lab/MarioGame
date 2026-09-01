#include "ui/UIUtils.h"
#include "ui/UIUtils.h"
#include "SaveManager/SaveManager.h"
#include "Level/Level.h"
#include "World/TileMap.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"
#include "AudioManager/AudioManager.h"
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

    void DrawCenteredTitle(const char* text, int y, int fontSize, Color color, int screenWidth) {
        Vector2 textSize = MeasureTextEx(Global::titleFont, text, (float)fontSize, 1.0f);
        DrawTextEx(Global::titleFont, text, { (screenWidth - textSize.x) / 2.0f, (float)y }, (float)fontSize, 1.0f, color);
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
        
        // Start menu music based on the loaded level's BGM
        AudioManager::PlayBGM(Level::GetLevel(currentBgLevel).GetBGMKey());
    }

    void UpdateMenuBackground(float dt) {
        if (!bgInitialized) return;
        
        // Start menu music only if nothing is currently playing
        if (AudioManager::CurrentBGM().empty()) {
            std::string expectedBGM = Level::GetLevel(currentBgLevel).GetBGMKey();
            AudioManager::PlayBGM(expectedBGM);
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

    }

    void DrawMenuHUD(float sw, float sh) {
        // Only draw the logo, remove MARIO, WORLD, TIME text.
        if (TextureManager::Has("title_logo")) {
            Texture2D& logo = TextureManager::Get("title_logo");
            float scale = (sh * 0.3f) / logo.height;
            float drawW = logo.width * scale;
            float drawH = logo.height * scale;
            float drawX = (sw - drawW) * 0.5f;
            float drawY = sh * 0.15f;
            DrawTextureEx(logo, {drawX, drawY}, 0.0f, scale, WHITE);
        } else {
            DrawCenteredTitle("SUPER MARIO", (int)(sh * 0.22f), (int)(sh * 0.08f), YELLOW, (int)sw);
        }
    }

    void CleanupMenuBackground() {
        bgInitialized = false;
    }

}
