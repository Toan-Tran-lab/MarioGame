#pragma once
#include "Player.h"

class Mario : public Player {
public:
    Mario() = default;
    
    const Animation* GetPoseAnimation() const override;
    const Animation* GetWalkAnimation() const override;
    const Animation* GetJumpAnimation() const override;
    const Animation* GetSlideAnimation() const override;
};
