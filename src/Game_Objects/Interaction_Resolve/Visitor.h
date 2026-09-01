#pragma once

class Goomba;
class KoopaShell;
class BuzzyBeetle;
class Player;
class Mushroom;
class FireFlower;
class Starman;
class KoopaShell;
class Boss;
class Piranha;
class Bullet;

class CharacterVisitor {
public:
    virtual ~CharacterVisitor() = default;
    virtual void Visit(Goomba& g) = 0;
    virtual void Visit(Player& p) = 0;
    virtual void Visit(KoopaShell& k) = 0;
    virtual void Visit(Mushroom& m) = 0;
    virtual void Visit(FireFlower& f) = 0;
    virtual void Visit(Starman& s) = 0;
    virtual void Visit(BuzzyBeetle& b) = 0;
    virtual void Visit(Boss& b) = 0;
    virtual void Visit(Piranha& p) = 0;
    virtual void Visit(Bullet& b) = 0;
};