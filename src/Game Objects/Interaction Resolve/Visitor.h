#pragma once

class Goomba;
class KoopaShell;
class Player;

class CharacterVisitor {
public:
    virtual ~visitor() = default;
    virtual void Visit(Goomba& g) = 0;
    virtual void Visit(Player& p) = 0;
    virtual void Visit(KoopaShell& k) = 0;
};