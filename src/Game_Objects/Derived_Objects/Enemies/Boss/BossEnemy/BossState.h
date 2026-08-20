#pragma once

class Boss;

class BossState {
public:
    virtual ~BossState() = default;
    virtual void Enter(Boss& boss) {}
    virtual void Exit(Boss& boss) {}
    virtual void UpdateState(Boss& boss, float dt) = 0;
};