#include "Mario.h"

// Normal Mario Animations (16x32)
static const Animation marioPoseAnim("mario_pose", 16, 32, 0, 2, {3.0f, 0.8f});
static const Animation marioWalkAnim("mario_walk", 16, 32, 0, 3, {0.1f});
static const Animation marioJumpAnim("mario_jump", 16, 32, 0, 1, {1.0f});
static const Animation marioSlideAnim("mario_slide", 16, 32, 0, 1, {1.0f});
static const Animation marioSitAnim("mario_sit", 16, 32, 0, 1, {1.0f});
static const Animation marioDieAnim("mario_pose", 16, 32, 0, 1, {1.0f}); // Placeholder for normal die

// Mini Mario Animations (16x16)
static const Animation marioMiniPoseAnim("mario_mini_pose", 16, 16, 0, 2, {3.0f, 0.8f});
static const Animation marioMiniWalkAnim("mario_mini_walk", 16, 16, 0, 3, {0.1f});
static const Animation marioMiniJumpAnim("mario_mini_jump", 16, 16, 0, 1, {1.0f});
static const Animation marioMiniSlideAnim("mario_mini_slide", 16, 16, 0, 1, {1.0f});
static const Animation marioMiniSitAnim("mario_mini_sit", 16, 16, 0, 1, {1.0f}); // if exists
static const Animation marioMiniDieAnim("mario_mini_die", 16, 16, 0, 1, {1.0f});

const Animation* Mario::GetPoseAnimation() const {
    return IsSmall() ? &marioMiniPoseAnim : &marioPoseAnim;
}

const Animation* Mario::GetWalkAnimation() const {
    return IsSmall() ? &marioMiniWalkAnim : &marioWalkAnim;
}

const Animation* Mario::GetJumpAnimation() const {
    return IsSmall() ? &marioMiniJumpAnim : &marioJumpAnim;
}

const Animation* Mario::GetSlideAnimation() const {
    return IsSmall() ? &marioMiniSlideAnim : &marioSlideAnim;
}

const Animation* Mario::GetSitAnimation() const {
    // If mini sit doesn't exist, we just return pose, but let's assume it exists or falls back
    return IsSmall() ? &marioMiniSitAnim : &marioSitAnim;
}

const Animation* Mario::GetDieAnimation() const {
    return IsSmall() ? &marioMiniDieAnim : &marioDieAnim;
}
