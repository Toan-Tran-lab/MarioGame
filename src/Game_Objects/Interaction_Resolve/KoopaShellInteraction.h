#pragma once
#include "Visitor.h"

class KoopaShell;

class ShellInteraction : public CharacterVisitor {
private:
    KoopaShell& self;
public:
    explicit ShellInteraction(KoopaShell& shell) : self(shell) {}

    void Visit(Goomba& g) override;
    void Visit(Player& p) override;
    void Visit(KoopaShell& k) override;
    void Visit(Mushroom& m) override;
};