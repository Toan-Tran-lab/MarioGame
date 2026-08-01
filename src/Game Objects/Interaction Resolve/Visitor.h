#pragma once
#include "BaseGameObjects.h"

class Goomba;
class KoopaShell;
class Player;

class CharacterVisitor {
public:
    virtual visitor() = default;
    virtual void Visit(Goomba& g) = 0;
    virtual void Visit(Player& p) = 0;
    virtual void Visit(KoopaShell& k) = 0;
}

class PlayerInteraction : public CharacterVisitor {
private:
    Player& self;
public:
    explicit PlayerInteraction(Player& other): self(other) {}

    void Visit(Goomba& g) override;
    void Visit(Player& p) override;
    void Visit(KoopaShell& k) override;
}