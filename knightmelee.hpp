#ifndef KNIGHTMELEE_H
#define KNIGHTMELEE_H

#include "entity.hpp"

#include "inugami/animatedsprite.hpp"

class KnightMelee : public Attack
{
public:
    KnightMelee() = delete;
    KnightMelee(const Coord& spos, bool sflip);
    virtual ~KnightMelee();

    virtual void tick() override;
    virtual void draw() override;

    virtual AABB getBB() const override;

    virtual Coord getPos() const override;
    virtual Coord getVel() const override;

    virtual void setPos(const Coord&) override;
    virtual void setVel(const Coord&) override;

    virtual void collide(Entity&) override;
    virtual bool isDead() const override;

    virtual bool isEnemyAttack() const override;
    virtual bool isMelee() const override;

private:
    Coord pos;
    bool dead;
    int life;
    Inugami::AnimatedSprite sprite;
};

#endif // KNIGHTMELEE_H
