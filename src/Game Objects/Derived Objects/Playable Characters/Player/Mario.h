#pragma once
#include "Player.h"

class Mario : public Player {
public:
    Mario() = default;
    
    Animation* CreatePoseAnimation() override;
    Animation* CreateWalkAnimation() override;
    Animation* CreateJumpAnimation() override;
    Animation* CreateSlideAnimation() override;
};
