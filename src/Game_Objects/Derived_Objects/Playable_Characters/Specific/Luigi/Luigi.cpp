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

// Fire Luigi Animations (updated to match re-exported PNG sizes)
static const Animation luigiFirePoseAnim("fire_luigi_pose", 90, 136, 0, 2, {3.0f, 0.8f}); // 180x136 / 2 = 90
static const Animation luigiFireWalkAnim("fire_luigi_walk", 90, 132, 0, 3, {0.1f}); // 270x132 / 3 = 90
static const Animation luigiFireJumpAnim("fire_luigi_jump", 78, 140, 0, 1, {1.0f});
static const Animation luigiFireSlideAnim("fire_luigi_slide", 96, 168, 0, 1, {1.0f});
static const Animation luigiFireSitAnim("fire_luigi_sit", 68, 106, 0, 1, {1.0f});
static const Animation luigiFireShootAnim("fire_luigi_shoot", 82, 140, 0, 1, {1.0f}); // 82x140

const Animation* Luigi::GetPoseAnimation() const {
    if (CanShootFireball()) return &luigiFirePoseAnim;
    return IsSmall() ? &luigiMiniPoseAnim : &luigiPoseAnim;
}

const Animation* Luigi::GetWalkAnimation() const {
    if (CanShootFireball()) return &luigiFireWalkAnim;
    return IsSmall() ? &luigiMiniWalkAnim : &luigiWalkAnim;
}

const Animation* Luigi::GetJumpAnimation() const {
    if (CanShootFireball()) return &luigiFireJumpAnim;
    return IsSmall() ? &luigiMiniJumpAnim : &luigiJumpAnim;
}

const Animation* Luigi::GetSlideAnimation() const {
    if (CanShootFireball()) return &luigiFireSlideAnim;
    return IsSmall() ? &luigiMiniSlideAnim : &luigiSlideAnim;
}

const Animation* Luigi::GetSitAnimation() const {
    if (CanShootFireball()) return &luigiFireSitAnim;
    return IsSmall() ? &luigiMiniSitAnim : &luigiSitAnim;
}

const Animation* Luigi::GetDieAnimation() const {
    return IsSmall() ? &luigiMiniDieAnim : &luigiDieAnim;
}

const Animation* Luigi::GetShootAnimation() const {
    if (CanShootFireball()) return &luigiFireShootAnim;
    return nullptr;
}
