#ifndef PLAYERRANGED_H
#define PLAYERRANGED_H

#include "entity.hpp"

#include "inugami/animatedsprite.hpp"

class PlayerRanged : public Attack
{
public:
    PlayerRanged() = delete;
    PlayerRanged(const Coord& spos, const Coord& svel, bool sflip);
    virtual ~PlayerRanged();

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
    virtual bool isRanged() const override;

private:
    Coord pos, vel;
    bool dead, flip;
    Inugami::AnimatedSprite sprite;
};

#endif // PLAYERRANGED_H
