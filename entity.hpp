#ifndef ENTITY_H
#define ENTITY_H

#include "aabb.hpp"
#include "coord.hpp"

class Entity
{
public:
    virtual ~Entity() = 0;

    virtual void tick();
    virtual void draw();

    virtual AABB getBB() const;

    virtual Coord getPos() const;
    virtual Coord getVel() const;

    virtual void setPos(const Coord&);
    virtual void setVel(const Coord&);

    virtual void collide(Entity&);
    virtual bool isDead() const;

    virtual bool isEnemy() const;
    virtual bool isEnemyAttack() const;
    virtual bool isPlayerAttack() const;
};

class Attack : public Entity
{
public:
    virtual ~Attack() = 0;
    virtual bool isMelee() const;
    virtual bool isRanged() const;
};

#endif // ENTITY_H
