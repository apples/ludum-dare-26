#ifndef SMOKEEFFECT_H
#define SMOKEEFFECT_H

#include "entity.hpp"
#include "coord.hpp"

#include "inugami/animatedsprite.hpp"

class SmokeEffect : public Entity
{
public:
    SmokeEffect() = delete;
    SmokeEffect(const Coord& spos);
    virtual ~SmokeEffect();

    virtual void tick() override;
    virtual void draw() override;

    virtual AABB getBB() const override;

    virtual Coord getPos() const override;
    virtual Coord getVel() const override;

    virtual void setPos(const Coord&) override;
    virtual void setVel(const Coord&) override;

    virtual void collide(Entity&) override;
    virtual bool isDead() const override;

private:
    Coord pos;
    bool dead;
    int life;
    Inugami::AnimatedSprite sprite;
};


#endif // SMOKEEFFECT_H
