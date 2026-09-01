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

// Fire Mario Animations (updated to match re-exported PNG sizes)
static const Animation marioFirePoseAnim("fire_mario_pose", 61, 94, 0, 2, {3.0f, 0.8f}); // 122x94 / 2 = 61
static const Animation marioFireWalkAnim("fire_mario_walk", 49, 82, 0, 3, {0.1f}); // 148x82 / 3 = 49 (same as before)
static const Animation marioFireJumpAnim("fire_mario_jump", 46, 84, 0, 1, {1.0f});
static const Animation marioFireSlideAnim("fire_mario_slide", 48, 90, 0, 1, {1.0f});
static const Animation marioFireSitAnim("fire_mario_sit", 82, 130, 0, 1, {1.0f});
static const Animation marioFireShootAnim("fire_mario_shoot", 61, 94, 0, 1, {1.0f}); // using pose sprite

const Animation* Mario::GetPoseAnimation() const {
    if (CanShootFireball()) return &marioFirePoseAnim;
    return IsSmall() ? &marioMiniPoseAnim : &marioPoseAnim;
}

const Animation* Mario::GetWalkAnimation() const {
    if (CanShootFireball()) return &marioFireWalkAnim;
    return IsSmall() ? &marioMiniWalkAnim : &marioWalkAnim;
}

const Animation* Mario::GetJumpAnimation() const {
    if (CanShootFireball()) return &marioFireJumpAnim;
    return IsSmall() ? &marioMiniJumpAnim : &marioJumpAnim;
}

const Animation* Mario::GetSlideAnimation() const {
    if (CanShootFireball()) return &marioFireSlideAnim;
    return IsSmall() ? &marioMiniSlideAnim : &marioSlideAnim;
}

const Animation* Mario::GetSitAnimation() const {
    if (CanShootFireball()) return &marioFireSitAnim;
    return IsSmall() ? &marioMiniSitAnim : &marioSitAnim;
}

const Animation* Mario::GetDieAnimation() const {
    return IsSmall() ? &marioMiniDieAnim : &marioDieAnim;
}

const Animation* Mario::GetShootAnimation() const {
    if (CanShootFireball()) return &marioFireShootAnim;
    return nullptr;
}
