#pragma once
#include "Visitor.h"

class PlayerInteraction : public CharacterVisitor {
private:
    Player& self;
public:
    explicit PlayerInteraction(Player& other): self(other) {}

    void Visit(Goomba& g) override;
    void Visit(Player& p) override;
    void Visit(KoopaShell& k) override;
    void Visit(Mushroom& m) override;
};