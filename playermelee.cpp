#include "playermelee.hpp"

#include "game.hpp"
#include "imagebank.hpp"
#include "smokeeffect.hpp"

#include "inugami/transform.hpp"

PlayerMelee::PlayerMelee(const Coord& spos, const Coord& svel, int sdir, bool sflip)
    : pos(spos)
    , vel(svel)
    , dead(false)
    , life(5)
    , dir(sdir)
    , sprite(ImageBank::getInstance().getSheet("player"))
{
    sprite.setSprites({{2,1}});
    sprite.setSequence({{0,12}});
    sprite.setMode(AnimatedSprite::Mode::LOOP);

    sprite.flipX = sflip;

    switch (dir)
    {
        case 1:
        {
            sprite.rot = 90.f;
        break;}

        case 2:
        {
            sprite.rot = 45.f;
        break;}

        case 3:
        {
            sprite.rot = -90.f;
        break;}

        case 4:
        {
            sprite.rot = -45.f;
        break;}
    }
}

PlayerMelee::~PlayerMelee()
{}

void PlayerMelee::tick()
{
    Game& core = Game::getInstance();

    for (auto& p : core.level->getCollisions(getBB()))
    {
        if (p.first->isBreakable())
        {
            core.level->setTile(p.second.left/8.0+0.5,p.second.bottom/8.0+0.5, 0);
            core.addEntity(new SmokeEffect(Coord(p.second.left+4.0,p.second.bottom+4.0)));
        }
    }

    pos += getVel();

    if (life>0) --life;
    if (life == 0) dead = true;
}

void PlayerMelee::draw()
{
    Game& core = Game::getInstance();

    Transform mat;
    mat.translate(Vec3{int(pos.x), int(pos.y), 0.f});
    core.modelMatrix(mat);

    sprite.draw(core, mat);
}

AABB PlayerMelee::getBB() const
{
    return AABB(-4,4,-4,4)+getPos();
}

Coord PlayerMelee::getPos() const
{
    return pos;
}

Coord PlayerMelee::getVel() const
{
    return vel;
}

void PlayerMelee::setPos(const Coord& in)
{
    pos = in;
}

void PlayerMelee::setVel(const Coord& in)
{
    vel = in;
}

void PlayerMelee::collide(Entity&)
{}

bool PlayerMelee::isDead() const
{
    return dead;
}

bool PlayerMelee::isPlayerAttack() const
{
    return true;
}

bool PlayerMelee::isMelee() const
{
    return true;
}
