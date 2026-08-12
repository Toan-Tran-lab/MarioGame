#include "Mario.h"

Animation* Mario::CreatePoseAnimation() {
    return new PoseAnimation("mario_pose");
}

Animation* Mario::CreateWalkAnimation() {
    return new WalkAnimation("mario_walk");
}

Animation* Mario::CreateJumpAnimation() {
    return new JumpAnimation("mario_jump");
}

Animation* Mario::CreateSlideAnimation() {
    return new SlideAnimation("mario_slide");
}
