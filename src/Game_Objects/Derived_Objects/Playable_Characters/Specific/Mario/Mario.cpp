#include "Mario.h"

// Define static animations for Mario
// Parameters: textureKey, tileW, tileH, startFrame, frameCount, frameDurations
static const Animation marioPoseAnim("mario_pose", 16, 30, 0, 2, {3.0f, 0.8f});
static const Animation marioWalkAnim("mario_walk", 16, 30, 0, 3, {0.1f});
static const Animation marioJumpAnim("mario_jump", 16, 30, 0, 1, {1.0f});
static const Animation marioSlideAnim("mario_slide", 16, 30, 0, 1, {1.0f});

const Animation* Mario::GetPoseAnimation() const {
    return &marioPoseAnim;
}

const Animation* Mario::GetWalkAnimation() const {
    return &marioWalkAnim;
}

const Animation* Mario::GetJumpAnimation() const {
    return &marioJumpAnim;
}

const Animation* Mario::GetSlideAnimation() const {
    return &marioSlideAnim;
}
