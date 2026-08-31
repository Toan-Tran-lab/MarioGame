#pragma once
#include "raylib.h"
#include "Game_Objects/Core_Header/BaseGameObjects.h"
#include <string>
#include <vector>

// Flyweight class containing shared animation data
struct Animation {
    std::string textureKey;
    int tileW;
    int tileH;
    int startFrame;
    int frameCount;
    std::vector<float> frameDurations; // Can have 1 element for uniform delay, or multiple for variable delays

    Animation() : tileW(0), tileH(0), startFrame(0), frameCount(0) {}
    Animation(const std::string& key, int tw, int th, int start, int count, const std::vector<float>& durations)
        : textureKey(key), tileW(tw), tileH(th), startFrame(start), frameCount(count), frameDurations(durations) {}

    float GetDuration(int frameOffset) const {
        if (frameDurations.empty()) return 0.1f;
        if (frameDurations.size() == 1) return frameDurations[0];
        return frameDurations[frameOffset % frameDurations.size()];
    }
};

// Extrinsic state class to be stored inside Game_Objects
class AnimationState {
private:
    const Animation* currentAnim = nullptr;
    float animTimer = 0.0f;
    int currentFrameOffset = 0;

public:
    void SetAnimation(const Animation* newAnim) {
        if (currentAnim != newAnim) {
            currentAnim = newAnim;
            animTimer = 0.0f;
            currentFrameOffset = 0;
        }
    }

    const Animation* GetAnimation() const { return currentAnim; }

    void Update(float dt);
    void Draw(const Vector2& position, FacingDirection facing, const Vector2& scale, Color tint = WHITE);
};
