#pragma once

class Goomba;
class KoopaShell;
class BuzzyBeetle;
class Player;
class Mushroom;

class CharacterVisitor {
public:
    virtual ~CharacterVisitor() = default;
    virtual void Visit(Goomba& g) = 0;
    virtual void Visit(Player& p) = 0;
    virtual void Visit(KoopaShell& k) = 0;
    virtual void Visit(Mushroom& m) = 0;
    virtual void Visit(BuzzyBeetle& b) = 0;
};