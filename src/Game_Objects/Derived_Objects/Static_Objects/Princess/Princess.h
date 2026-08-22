#pragma once
#include "Game_Objects/Core_Header/BaseGameObjects.h"
#include "Animations/Animation.h"

// Stationary, non-interactive objective marker. Doesn't move, doesn't take
// damage, doesn't participate in the CharacterVisitor combat system —
// GameplayState detects proximity to her directly and triggers the ending.
class Princess : public GameObject {
private:
    AnimationState animState;
    float interactionRadius_ = 56.0f; // tune to taste — center-to-center trigger distance

public:
    Princess();

    float GetInteractionRadius() const { return interactionRadius_; }
    void SetInteractionRadius(float radius) { interactionRadius_ = radius; }

    void Update(float dt) override; // idle animation only — no gameplay logic
    void Draw() override;
};