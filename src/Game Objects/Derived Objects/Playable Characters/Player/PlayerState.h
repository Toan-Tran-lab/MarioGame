#pragma once

class Player;

enum class PowerupType {
    Mushroom,
    FireFlower,
    Star
};

class PlayerState {
public:
    virtual ~PlayerState() = default;
    virtual void Enter(Player& player) {}
    virtual void Exit(Player& player) {}
    virtual void OnHit(Player& player) = 0;      // took damage
    virtual void OnPowerup(Player& player, PowerupType type) = 0; // collected mushroom/flower
    virtual void UpdateState(Player& player, float dt);
};

class SmallState : public PlayerState {
public:
    void Enter(Player& player) override;
    void OnHit(Player& player) override;      // already smallest -> trigger death/respawn
    void OnPowerup(Player& player, PowerupType type) override; // -> transition to SuperState
};

class SuperState : public PlayerState {
public:
    void Enter(Player& player) override;      // swap sprite/hitbox to "big" size
    void OnHit(Player& player) override;      // -> transition back to SmallState
    void OnPowerup(Player& player, PowerupType type) override; // fire flower -> FireState
};

class FireState : public PlayerState {
public:
    void OnHit(Player& player) override;      // classic Mario rule: Fire -> Small directly, skips Super
    void OnPowerup(Player& player, PowerupType type) override; // already max state; Star still applies
};

class StarState : public PlayerState {
public:
    void Enter(Player& player) override;
    void OnHit(Player& player) override;      // classic Mario rule: Fire -> Small directly, skips Super
    void OnPowerup(Player& player, PowerupType type) override {} // already max state
};

// --- TransformingState ---
class TransformingState : public PlayerState {
private:
    PlayerState* nextState_;
    float timer_;
    int blinkCount_;
    bool isGrowing_;
    bool currentIsSmall_;

public:
    TransformingState(PlayerState* nextState, bool isGrowing);
    ~TransformingState() override;
    
    void Enter(Player& player) override;
    void UpdateState(Player& player, float dt) override;
    void OnHit(Player& player) override;
    void OnPowerup(Player& player, PowerupType type) override {}
};