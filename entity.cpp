#include "entity.hpp"

Entity::~Entity()
{}

void Entity::tick()
{}

void Entity::draw()
{}

AABB Entity::getBB() const
{
    return AABB(0,0,0,0);
}

Coord Entity::getPos() const
{
    return Coord(0,0);
}

Coord Entity::getVel() const
{
    return Coord(0,0);
}

void Entity::setPos(const Coord&)
{}

void Entity::setVel(const Coord&)
{}

void Entity::collide(Entity&)
{}

bool Entity::isDead() const
{
    return false;
}

bool Entity::isEnemy() const
{
    return false;
}

bool Entity::isEnemyAttack() const
{
    return false;
}

bool Entity::isPlayerAttack() const
{
    return false;
}

Attack::~Attack()
{}

bool Attack::isMelee() const
{
    return false;
}

bool Attack::isRanged() const
{
    return false;
}
