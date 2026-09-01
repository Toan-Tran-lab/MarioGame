#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"

enum class PiranhaState {
    Hidden,     // Fully retracted inside pipe / ground
    Rising,     // Emerging upward
    Exposed,    // Fully emerged, actively chomping
    Retracting  // Sinking downward
};

class Piranha : public Character {
private:
    PiranhaState state_ = PiranhaState::Hidden;
    AnimationState animState;

    Vector2 baseSpawnPos_ = {0.0f, 0.0f};
    bool spawnRecorded_ = false;

    float timer_ = 0.0f;
    static constexpr float kHiddenDuration = 2.0f;   // Seconds to stay hidden
    static constexpr float kExposedDuration = 2.0f;  // Seconds to stay exposed
    static constexpr float kMoveSpeed = 60.0f;        // Pixels per second to rise/sink

    const physics::PhysicsBody* playerBody_ = nullptr;

public:
    Piranha();
    ~Piranha() override = default;

    PiranhaState GetState() const { return state_; }
    bool IsExposedOrMoving() const { return state_ != PiranhaState::Hidden; }

    void SetPlayerBody(const physics::PhysicsBody* player) { playerBody_ = player; }

    void InteractWith(Character& other) override { (void)other; }
    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;
};
