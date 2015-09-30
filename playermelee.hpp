#ifndef PLAYERMELEE_H
#define PLAYERMELEE_H

#include "entity.hpp"

#include "inugami/animatedsprite.hpp"

class PlayerMelee : public Attack
{
public:
    PlayerMelee() = delete;
    PlayerMelee(const Coord& spos, const Coord& svel, int sdir, bool sflip);
    virtual ~PlayerMelee();

    virtual void tick() override;
    virtual void draw() override;

    virtual AABB getBB() const override;

    virtual Coord getPos() const override;
    virtual Coord getVel() const override;

    virtual void setPos(const Coord&) override;
    virtual void setVel(const Coord&) override;

    virtual void collide(Entity&) override;
    virtual bool isDead() const override;

    virtual bool isPlayerAttack() const override;
    virtual bool isMelee() const override;

private:
    Coord pos, vel;
    bool dead;
    int life, dir;
    Inugami::AnimatedSprite sprite;
};

#endif // PLAYERMELEE_H
