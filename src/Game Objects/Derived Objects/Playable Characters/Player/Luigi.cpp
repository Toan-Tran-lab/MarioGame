#include "Luigi.h"

// Define static animations for Luigi
// Parameters: textureKey, tileW, tileH, startFrame, frameCount, frameDurations
static const Animation luigiPoseAnim("luigi_pose", 16, 30, 0, 2, {3.0f, 0.8f});
static const Animation luigiWalkAnim("luigi_walk", 16, 30, 0, 3, {0.1f});
static const Animation luigiJumpAnim("luigi_jump", 16, 30, 0, 1, {1.0f});
static const Animation luigiSlideAnim("luigi_slide", 16, 30, 0, 1, {1.0f});

const Animation* Luigi::GetPoseAnimation() const {
    return &luigiPoseAnim;
}

const Animation* Luigi::GetWalkAnimation() const {
    return &luigiWalkAnim;
}

const Animation* Luigi::GetJumpAnimation() const {
    return &luigiJumpAnim;
}

const Animation* Luigi::GetSlideAnimation() const {
    return &luigiSlideAnim;
}
