#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"
#include "World/BlockGrid.h"
#include "TextureManager/TextureManager.h"
#include <vector>

// Manages the flagpole assembly (solid pole + sliding flag)
// The pole is a vertical stack of FlagPole entities from LDtk.
// The flag is a single Flag entity tile that slides down when triggered.
class Flagpole {
public:
    enum class State { Idle, FlagDescending, MarioSliding, Complete };

private:
    // Pole geometry (world coordinates, already scaled)
    float poleX_ = 0.0f;
    float poleTop_ = 0.0f;       // topmost Y of the pole
    float poleBottom_ = 0.0f;    // bottommost Y + tileSize
    std::vector<Vector2> poleSegments_; // positions of each FlagPole entity

    // Flag
    float flagY_ = 0.0f;         // current Y position of the flag (animated)
    float flagStartY_ = 0.0f;    // initial Y (top of pole)
    float flagEndY_ = 0.0f;      // destination Y (bottom of pole - 1 tile)
    Rectangle flagSrcRect_ = { 612.0f, 561.0f, 16.0f, 16.0f }; // source rect in tileset.png
    bool hasFlag_ = false;

    // Display
    float tileSize_ = 48.0f;     // display tile size
    float scale_ = 3.0f;         // tileSize / gridSize

    // State
    State state_ = State::Idle;
    float flagSpeed_ = 200.0f;   // pixels per second descent speed

    bool initialized_ = false;

public:
    Flagpole() = default;

    // Called during init for each FlagPole entity from LDtk
    void AddPoleSegment(Vector2 worldPos);

    // Called during init for the Flag entity from LDtk
    void SetFlagPosition(Vector2 worldPos);

    // Insert solid TerrainBlocks into the BlockGrid for each pole segment
    void InsertSolidBlocks(BlockGrid& grid, int tileSize);

    // Finalize after all segments and flag have been added
    void Finalize();

    // Returns a trigger bounding rect (slightly wider than the pole) for overlap detection
    Rectangle GetTriggerBounds() const;

    // Start the flag descent animation (flag starts from its default position)
    void Trigger();

    // Start the flag descent from the Y coordinate where Mario hit the pole
    void Trigger(float marioY);

    // Returns true when the flag has finished descending
    bool IsComplete() const;

    // Returns true when Mario+flag are currently sliding down
    bool IsSliding() const { return state_ == State::FlagDescending || state_ == State::MarioSliding; }

    // Getters for GameplayState to position Mario during the slide
    float GetFlagY() const { return flagY_; }
    float GetPoleX() const { return poleX_; }
    float GetPoleBottom() const { return poleBottom_; }

    // Returns true if any pole segments have been added
    bool HasPole() const { return !poleSegments_.empty(); }

    void Update(float dt);
    void Draw();
};
