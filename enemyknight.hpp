#ifndef ENEMYKNIGHT_H
#define ENEMYKNIGHT_H

#include "entity.hpp"

#include "inugami/animatedsprite.hpp"

#include <string>
#include <map>

class EnemyKnight : public Entity
{
public:
    EnemyKnight();
    virtual ~EnemyKnight();

    virtual void tick() override;
    virtual void draw() override;

    virtual AABB getBB() const override;

    virtual Coord getPos() const override;
    virtual Coord getVel() const override;

    virtual void setPos(const Coord&) override;
    virtual void setVel(const Coord&) override;

    virtual void collide(Entity&) override;
    virtual bool isDead() const override;

    virtual bool isEnemy() const override;

private:
    void die();
    bool canSeePlayer() const;

    Coord pos, vel;
    bool dead, grounded, flipX, walking, defending, ready;
    int wallSlide, wanderTimer, attackTimer;
    std::string curSprite;
    std::map<std::string,Inugami::AnimatedSprite*> sprites;
};

#endif // ENEMYKNIGHT_H
