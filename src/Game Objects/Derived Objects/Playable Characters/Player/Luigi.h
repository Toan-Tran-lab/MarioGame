#pragma once
#include "Player.h"

class Luigi : public Player {
public:
    Luigi() = default;
    
    Animation* CreatePoseAnimation() override;
    Animation* CreateWalkAnimation() override;
    Animation* CreateJumpAnimation() override;
    Animation* CreateSlideAnimation() override;
};
