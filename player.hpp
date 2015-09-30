#ifndef PLAYER_H
#define PLAYER_H

#include "entity.hpp"

#include "inugami/animatedsprite.hpp"
#include "inugami/interface.hpp"

#include <map>
#include <string>

class Player : public Entity
{
public:
    Player();
    virtual ~Player();

    virtual void tick() override;
    virtual void draw() override;

    virtual AABB getBB() const override;

    virtual Coord getPos() const override;
    virtual Coord getVel() const override;

    virtual void setPos(const Coord& in) override;
    virtual void setVel(const Coord& in) override;

    virtual void collide(Entity& ent) override;
    virtual bool isDead() const override;

    void die();

private:
    Coord pos;
    Coord vel;

    bool grounded, jumpReady, flipX, gliding;
    int wallSlide;
    int attackTimer;
    std::string curSprite;

    std::map<std::string,Inugami::AnimatedSprite*> sprites;
    std::map<std::string,Inugami::Interface::Proxy> keys;

    bool dieing, dead;
};

#endif // PLAYER_H
