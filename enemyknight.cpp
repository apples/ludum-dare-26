#include "enemyknight.hpp"

#include "game.hpp"
#include "imagebank.hpp"
#include "smokeeffect.hpp"
#include "knightmelee.hpp"

#include "inugami/image.hpp"
#include "inugami/spritesheet.hpp"
#include "inugami/utility.hpp"

#include <utility>
#include <vector>
#include <functional>
#include <random>

using namespace Inugami;
using namespace std;

EnemyKnight::EnemyKnight()
    : pos()
    , vel()
    , dead(false)
    , grounded(false)
    , flipX(false)
    , walking(false)
    , defending(false)
    , ready(false)
    , wallSlide(0)
    , wanderTimer(0)
    , attackTimer(0)
    , curSprite("idle")
    , sprites()
{
    Game& core = Game::getInstance();

    Spritesheet& sheet8x8 = ImageBank::getInstance().getSheet("enemies");
    AnimatedSprite* sprite = nullptr;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{1,0}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["idle"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{2,0},{2,1}});
    sprite->setSequence({{0,12},{1,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["walk"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{3,0}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["defend"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{3,1}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["ready"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{3,2}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["attack"] = sprite;
}

EnemyKnight::~EnemyKnight()
{
    for (auto& p : sprites) delete p.second;
}

void EnemyKnight::tick()
{
    Game& core = Game::getInstance();

    static constexpr double gravity = 0.15;
    static constexpr double friction = 0.5;
    static constexpr double walkacc = 0.5;
    static constexpr double airfriction = 0.9;
    static constexpr double maxFallSpeed = 2.5;

    if (--wanderTimer <= 0 && !ready)
    {
        std::uniform_int_distribution<int> rollA(0,1);
        std::uniform_int_distribution<int> rollB(0,60*2);

        walking = rollA(core.rng);
        flipX = rollA(core.rng);
        wanderTimer = rollB(core.rng);
    }

    curSprite = "idle";

    defending = false;

    if (canSeePlayer())
    {
        defending = true;
        flipX = (core.player->getPos().x < getPos().x);

        if (!ready)
        {
            if (abs(core.player->getPos().x - getPos().x) <= 16.0)
            {
                ready = true;
                attackTimer = 30;
            }
        }
    }
    else
    {
        if (ready && attackTimer) attackTimer = 0;
        defending = false;
        ready = false;
    }

    if (defending) curSprite = "defend";

    if (attackTimer > 0) --attackTimer, curSprite = "attack";
    if (attackTimer < 0) attackTimer = 0;

    if (ready)
    {
        curSprite = "ready";
        defending = false;

        if (attackTimer <= 0)
        {
            ready = false;
            attackTimer = 3;

            Coord apos(8,0);

            if (flipX) apos.x *= -1;

            core.addEntity(new KnightMelee(getPos()+apos, flipX));
        }
    }


    if (walking && !(defending||ready||attackTimer))
    {
        curSprite = "walk";
        if (flipX)
        {
            vel.x -= (grounded?walkacc:0);
        }
        else
        {
            vel.x += (grounded?walkacc:0);
        }
    }

    if (grounded) vel.x *= friction;
    else vel.x *= airfriction;

    AABB current = getBB();
    AABB target;

    grounded = false;

    if (vel.x != 0.0) wallSlide = 0;

    auto   intify  = [&](double d){return int(d/8.0);};
    auto   intifyr = [&](double d){return core.level->width -int(core.level->width -d/8.0)-1;};
    auto   intifyt = [&](double d){return core.level->height-int(core.level->height-d/8.0)-1;};
    auto deintify  = [&](   int i){return double(i*8.0);};

    int itleft;
    int itright;
    int itbottom;
    int ittop;

    int icleft   = intify (current.left  );
    int icright  = intifyr(current.right );
    int icbottom = intify (current.bottom);
    int ictop    = intifyt(current.top   );

    auto recalc = [&]
    {
        target   = current + vel;
        itleft   = intify (target.left  );
        itright  = intifyr(target.right );
        itbottom = intify (target.bottom);
        ittop    = intifyt(target.top   );
    };

    recalc();

    if (vel.y < 0.0)
    {
        if (itbottom != icbottom)
        {
            const Tile& ac = core.level->tileAt(itleft , itbottom  );
            const Tile& an = core.level->tileAt(itleft , itbottom+1);
            const Tile& bc = core.level->tileAt(itright, itbottom  );
            const Tile& bn = core.level->tileAt(itright, itbottom+1);

            if ((ac.isSolid() && !an.isSolid()) || (bc.isSolid() && !bn.isSolid()))
            {
                vel.y = icbottom*8.0-current.bottom;
                grounded = true;
            }
        }
    }

    recalc();

    if (vel.x < 0.0)
    {
        if (itleft != icleft)
        {
            const Tile& ac = core.level->tileAt(itleft  , itbottom);
            const Tile& an = core.level->tileAt(itleft+1, itbottom);
            const Tile& bc = core.level->tileAt(itleft  , ittop   );
            const Tile& bn = core.level->tileAt(itleft+1, ittop   );

            const Tile& nextFloor = core.level->tileAt(itleft, icbottom-1);
            const Tile& nextWall  = core.level->tileAt(itleft, icbottom);

            if (!nextFloor.isSolid()
             || nextWall.isDeadly()
             || (ac.isSolid() && !an.isSolid())
             || (bc.isSolid() && !bn.isSolid()))
            {
                vel.x = 0;
                wallSlide = 1;
            }
        }
    }

    recalc();

    if (vel.y > 0.0)
    {
        if (ittop != ictop)
        {
            const Tile& ac = core.level->tileAt(itleft , ittop);
            const Tile& an = core.level->tileAt(itleft , ittop-1);
            const Tile& bc = core.level->tileAt(itright, ittop);
            const Tile& bn = core.level->tileAt(itright, ittop-1);

            if ((ac.isSolid() && !an.isSolid()) || (bc.isSolid() && !bn.isSolid()))
            {
                vel.y = (ictop+1)*8.0-current.top;
            }
        }
    }

    recalc();

    if (vel.x > 0.0)
    {
        if (itright != icright)
        {
            const Tile& ac = core.level->tileAt(itright  , itbottom);
            const Tile& an = core.level->tileAt(itright-1, itbottom);
            const Tile& bc = core.level->tileAt(itright  , ittop);
            const Tile& bn = core.level->tileAt(itright-1, ittop);

            const Tile& nextFloor = core.level->tileAt(itright, icbottom-1);
            const Tile& nextWall  = core.level->tileAt(itright, icbottom);

            if (!nextFloor.isSolid()
             || nextWall.isDeadly()
             || (ac.isSolid() && !an.isSolid())
             || (bc.isSolid() && !bn.isSolid()))
            {
                vel.x = 0;
                wallSlide = -1;
            }
        }
    }

    if (wallSlide != 0) flipX = !flipX;

    if (grounded) wallSlide = 0;

    pos += vel;

    for (auto& p : core.level->getCollisions(getBB())) if (p.first->isDeadly()) {die(); break;}

    vel.y -= gravity;
    if (vel.y < -maxFallSpeed) vel.y = -maxFallSpeed;
}

void EnemyKnight::draw()
{
    Game& core = Game::getInstance();

    Transform mat;
    mat.translate(Vec3{int(pos.x), int(pos.y), 0.f});
    core.modelMatrix(mat);

    auto spr = sprites[curSprite];
    spr->flipX = flipX;
    spr->draw(core, mat);
}

AABB EnemyKnight::getBB() const
{
    return AABB(-3.0, 3.0, -4.0, 4.0)+pos;
}

Coord EnemyKnight::getPos() const
{
    return pos;
}

Coord EnemyKnight::getVel() const
{
    return vel;
}

void EnemyKnight::setPos(const Coord& in)
{
    pos = in;
}

void EnemyKnight::setVel(const Coord& in)
{
    vel = in;
}

void EnemyKnight::collide(Entity& ent)
{
    if (ent.isPlayerAttack())
    {
        Attack& atk = reinterpret_cast<Attack&>(ent);
        if (atk.isMelee()) die();
    }
}

bool EnemyKnight::isDead() const
{
    return dead;
}

bool EnemyKnight::isEnemy() const
{
    return true;
}

void EnemyKnight::die()
{
    Game& core = Game::getInstance();
    dead = true;
    core.addEntity(new SmokeEffect(getPos()));
}

bool EnemyKnight::canSeePlayer() const
{
    Game& core = Game::getInstance();

    if (int(core.player->getPos().y/8.0)==int(getPos().y/8.0)
     && abs(core.player->getPos().x - getPos().x) <= 80.0)
    {
        int y = int(getPos().y/8.0);
        int x = min(int(getPos().x/8.0), int(core.player->getPos().x/8.0));
        int xd = max(int(getPos().x/8.0), int(core.player->getPos().x/8.0));
        for (; x<xd; ++x) if (core.level->tileAt(x,y).isSolid()) break;
        if (x == xd) return true;
    }

    return false;
}
