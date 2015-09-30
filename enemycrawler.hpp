#ifndef ENEMYCRAWLER_H
#define ENEMYCRAWLER_H

#include "entity.hpp"

#include "inugami/animatedsprite.hpp"

#include <string>
#include <map>

class EnemyCrawler : public Entity
{
public:
    EnemyCrawler();
    virtual ~EnemyCrawler();

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

    Coord pos, vel;
    bool dead, grounded, flipX;
    int wallSlide;
    std::string curSprite;
    std::map<std::string,Inugami::AnimatedSprite*> sprites;
};

#endif // ENEMYCRAWLER_H
