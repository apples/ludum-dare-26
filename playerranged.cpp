#include "playerranged.hpp"
#include "playermelee.hpp"

#include "game.hpp"
#include "imagebank.hpp"
#include "smokeeffect.hpp"

#include "inugami/transform.hpp"

#include <random>

using namespace std;

PlayerRanged::PlayerRanged(const Coord& spos, const Coord& svel, bool sflip)
    : pos(spos)
    , vel(svel)
    , dead(false)
    , flip(sflip)
    , sprite(ImageBank::getInstance().getSheet("player"))
{
    Game& core = Game::getInstance();
    uniform_real_distribution<double> roll(0.0,1.0);
    if (roll(core.rng) > 0.975) sprite.setSprites({{2,4},{2,5}});
    else sprite.setSprites({{2,2},{2,3}});
    sprite.setSequence({{0,6},{1,6}});
    sprite.setMode(AnimatedSprite::Mode::LOOP);
}

PlayerRanged::~PlayerRanged()
{}

void PlayerRanged::tick()
{
    Game& core = Game::getInstance();

    pos += getVel();

    for (auto& p : core.level->getCollisions(getBB()))
    {
        if (p.first->isBreakable())
        {
            core.level->setTile(p.second.left/8.0+0.5,p.second.bottom/8.0+0.5, 0);
            core.addEntity(new SmokeEffect(Coord(p.second.left+4.0,p.second.bottom+4.0)));
            dead = true;
            continue;
        }
        if (p.first->isSolid())
        {
            dead = true;
            continue;
        }
    }
}

void PlayerRanged::draw()
{
    Game& core = Game::getInstance();

    Transform mat;
    mat.translate(Vec3{int(pos.x), int(pos.y), 0.f});
    core.modelMatrix(mat);

    sprite.draw(core, mat);
}

AABB PlayerRanged::getBB() const
{
    return AABB(-2,2,-2,2)+getPos();
}

Coord PlayerRanged::getPos() const
{
    return pos;
}

Coord PlayerRanged::getVel() const
{
    return vel;
}

void PlayerRanged::setPos(const Coord& in)
{
    pos = in;
}

void PlayerRanged::setVel(const Coord&)
{}

void PlayerRanged::collide(Entity& ent)
{
    if (ent.isEnemy())
    {
        dead = true;
    }
}

bool PlayerRanged::isDead() const
{
    return dead;
}

bool PlayerRanged::isPlayerAttack() const
{
    return true;
}

bool PlayerRanged::isRanged() const
{
    return true;
}
