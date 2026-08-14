#pragma once
#include "Player.h"

class Luigi : public Player {
public:
    Luigi() = default;
    
    const Animation* GetPoseAnimation() const override;
    const Animation* GetWalkAnimation() const override;
    const Animation* GetJumpAnimation() const override;
    const Animation* GetSlideAnimation() const override;
};
