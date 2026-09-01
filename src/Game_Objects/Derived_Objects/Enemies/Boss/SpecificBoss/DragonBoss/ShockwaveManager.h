#pragma once
#include "raylib.h"
#include <vector>
#include <unordered_set>

class BlockGrid;
class Player;

struct GroundShockwave {
    Vector2 origin{ 0.0f, 0.0f };
    float currentRadius = 0.0f;
    float maxRadius = 750.0f;
    float waveSpeed = 420.0f; // pixels per second
    float floorY = 0.0f;
    bool active = true;
    std::unordered_set<int> bumpedCols;
    std::unordered_set<const Player*> hitPlayers;
};

class ShockwaveManager {
private:
    std::vector<GroundShockwave> waves_;

public:
    void Trigger(Vector2 origin, float floorY, float maxRadius = 750.0f, float speed = 420.0f);
    void Update(float dt, BlockGrid& blockGrid, const std::vector<Player*>& players);
    void Draw() const;
    void Clear();
    bool HasActiveWaves() const;
};
