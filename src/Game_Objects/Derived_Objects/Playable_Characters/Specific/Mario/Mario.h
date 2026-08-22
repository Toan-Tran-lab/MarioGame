#pragma once
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"

class Mario : public Player {
public:
    Mario() = default;

    //Attributes Override
    float GetMoveSpeedMultiplier() const override { return 1.25f; } //Faster Sprinting
    float GetSkidDecel() const override { return 300.0f; }   // Better turning
    
    //Annimation Overrdide
    const Animation* GetPoseAnimation() const override;
    const Animation* GetWalkAnimation() const override;
    const Animation* GetJumpAnimation() const override;
    const Animation* GetSlideAnimation() const override;
    const Animation* GetSitAnimation() const override;
    const Animation* GetDieAnimation() const override;
};
