#pragma once
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"

class Luigi : public Player {
public:
    Luigi() = default;

    //Attribute Override
    float GetJumpForce()         const override { return -750.0f; } // jumps a bit higher
    float GetGravityMultiplier() const override { return 0.9f; }    // floatier fall
    
    //Animation Override
    const Animation* GetPoseAnimation() const override;
    const Animation* GetWalkAnimation() const override;
    const Animation* GetJumpAnimation() const override;
    const Animation* GetSlideAnimation() const override;
    const Animation* GetSitAnimation() const override;
    const Animation* GetDieAnimation() const override;
};
