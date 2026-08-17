#include "Luigi.h"

// Normal Luigi Animations (16x32)
static const Animation luigiPoseAnim("luigi_pose", 16, 32, 0, 2, {3.0f, 0.8f});
static const Animation luigiWalkAnim("luigi_walk", 16, 32, 0, 3, {0.1f});
static const Animation luigiJumpAnim("luigi_jump", 16, 32, 0, 1, {1.0f});
static const Animation luigiSlideAnim("luigi_slide", 16, 32, 0, 1, {1.0f});
static const Animation luigiSitAnim("luigi_sit", 16, 32, 0, 1, {1.0f});
static const Animation luigiDieAnim("luigi_pose", 16, 32, 0, 1, {1.0f}); // Placeholder for normal die

// Mini Luigi Animations (16x16)
static const Animation luigiMiniPoseAnim("luigi_mini_pose", 16, 16, 0, 2, {3.0f, 0.8f});
static const Animation luigiMiniWalkAnim("luigi_mini_walk", 16, 16, 0, 3, {0.1f});
static const Animation luigiMiniJumpAnim("luigi_mini_jump", 16, 16, 0, 1, {1.0f});
static const Animation luigiMiniSlideAnim("luigi_mini_slide", 16, 16, 0, 1, {1.0f});
static const Animation luigiMiniSitAnim("luigi_mini_sit", 16, 16, 0, 1, {1.0f});
static const Animation luigiMiniDieAnim("luigi_mini_die", 16, 16, 0, 1, {1.0f});

const Animation* Luigi::GetPoseAnimation() const {
    return IsSmall() ? &luigiMiniPoseAnim : &luigiPoseAnim;
}

const Animation* Luigi::GetWalkAnimation() const {
    return IsSmall() ? &luigiMiniWalkAnim : &luigiWalkAnim;
}

const Animation* Luigi::GetJumpAnimation() const {
    return IsSmall() ? &luigiMiniJumpAnim : &luigiJumpAnim;
}

const Animation* Luigi::GetSlideAnimation() const {
    return IsSmall() ? &luigiMiniSlideAnim : &luigiSlideAnim;
}

const Animation* Luigi::GetSitAnimation() const {
    return IsSmall() ? &luigiMiniSitAnim : &luigiSitAnim;
}

const Animation* Luigi::GetDieAnimation() const {
    return IsSmall() ? &luigiMiniDieAnim : &luigiDieAnim;
}
