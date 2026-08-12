#include "Luigi.h"

Animation* Luigi::CreatePoseAnimation() {
    return new PoseAnimation("luigi_pose");
}

Animation* Luigi::CreateWalkAnimation() {
    return new WalkAnimation("luigi_walk");
}

Animation* Luigi::CreateJumpAnimation() {
    return new JumpAnimation("luigi_jump");
}

Animation* Luigi::CreateSlideAnimation() {
    return new SlideAnimation("luigi_slide");
}
