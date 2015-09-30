#include "smokeeffect.hpp"

#include "game.hpp"
#include "imagebank.hpp"

#include "inugami/transform.hpp"

#include <random>

using namespace std;

SmokeEffect::SmokeEffect(const Coord& spos)
    : pos(spos)
    , dead(false)
    , life(20)
    , sprite(ImageBank::getInstance().getSheet("effects"))
{
    Game& core = Game::getInstance();
    uniform_int_distribution<int> roll(0,1);

    int row = roll(core.rng);

    sprite.setSprites({{row,0},{row,1},{row,2}});
    sprite.setSequence({{0,6},{1,6},{2,6}});
    sprite.setMode(AnimatedSprite::Mode::LOOP);
}

SmokeEffect::~SmokeEffect()
{}

void SmokeEffect::tick()
{
    if (life>0) --life;
    if (life == 0) dead = true;
}

void SmokeEffect::draw()
{
    Game& core = Game::getInstance();

    Transform mat;
    mat.translate(Vec3{int(pos.x), int(pos.y), 0.f});
    core.modelMatrix(mat);

    sprite.draw(core, mat);
}

AABB SmokeEffect::getBB() const
{
    return AABB(-4,4,-4,4)+getPos();
}

Coord SmokeEffect::getPos() const
{
    return pos;
}

Coord SmokeEffect::getVel() const
{
    return Coord(0,0);
}

void SmokeEffect::setPos(const Coord& in)
{
    pos = in;
}

void SmokeEffect::setVel(const Coord&)
{}

void SmokeEffect::collide(Entity&)
{}

bool SmokeEffect::isDead() const
{
    return dead;
}
