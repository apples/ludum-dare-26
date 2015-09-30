#include "knightmelee.hpp"

#include "game.hpp"
#include "imagebank.hpp"
#include "smokeeffect.hpp"

#include "inugami/transform.hpp"

KnightMelee::KnightMelee(const Coord& spos, bool sflip)
    : pos(spos)
    , dead(false)
    , life(5)
    , sprite(ImageBank::getInstance().getSheet("enemies"))
{
    sprite.setSprites({{3,3}});
    sprite.setSequence({{0,12}});
    sprite.setMode(AnimatedSprite::Mode::LOOP);

    sprite.flipX = sflip;
}

KnightMelee::~KnightMelee()
{}

void KnightMelee::tick()
{
    if (life>0) --life;
    if (life == 0) dead = true;
}

void KnightMelee::draw()
{
    Game& core = Game::getInstance();

    Transform mat;
    mat.translate(Vec3{int(pos.x), int(pos.y), 0.f});
    core.modelMatrix(mat);

    sprite.draw(core, mat);
}

AABB KnightMelee::getBB() const
{
    return AABB(-4,4,-4,4)+getPos();
}

Coord KnightMelee::getPos() const
{
    return pos;
}

Coord KnightMelee::getVel() const
{
    return Coord(0,0);
}

void KnightMelee::setPos(const Coord& in)
{
    pos = in;
}

void KnightMelee::setVel(const Coord&)
{}

void KnightMelee::collide(Entity&)
{}

bool KnightMelee::isDead() const
{
    return dead;
}

bool KnightMelee::isEnemyAttack() const
{
    return true;
}

bool KnightMelee::isMelee() const
{
    return true;
}
