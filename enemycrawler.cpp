#include "enemycrawler.hpp"

#include "game.hpp"
#include "imagebank.hpp"
#include "smokeeffect.hpp"

#include "inugami/image.hpp"
#include "inugami/spritesheet.hpp"
#include "inugami/utility.hpp"

#include <utility>
#include <vector>
#include <functional>

using namespace Inugami;
using namespace std;

EnemyCrawler::EnemyCrawler()
    : pos()
    , vel()
    , grounded(false)
    , flipX(false)
    , wallSlide(0)
    , curSprite("move")
    , sprites()
    , dead(false)
{
    Game& core = Game::getInstance();

    Spritesheet& sheet8x8 = ImageBank::getInstance().getSheet("enemies");
    AnimatedSprite* sprite = nullptr;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{0,0},{0,1}});
    sprite->setSequence({{0,12},{1,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["move"] = sprite;
}

EnemyCrawler::~EnemyCrawler()
{
    for (auto& p : sprites) delete p.second;
}

void EnemyCrawler::tick()
{
    Game& core = Game::getInstance();

    static constexpr double gravity = 0.15;
    static constexpr double friction = 0.5;
    static constexpr double walkacc = 0.5;
    static constexpr double airfriction = 0.9;
    static constexpr double maxFallSpeed = 2.5;

    curSprite = "move";

    if (flipX)
    {
        vel.x -= (grounded?walkacc:0);
    }
    else
    {
        vel.x += (grounded?walkacc:0);
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
                vel.x = icleft*8.0-current.left+0.1;
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
                vel.x = (icright+1)*8.0-current.right-0.1;
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

void EnemyCrawler::draw()
{
    Game& core = Game::getInstance();

    Transform mat;
    mat.translate(Vec3{int(pos.x), int(pos.y), 0.f});
    core.modelMatrix(mat);

    auto spr = sprites[curSprite];
    spr->flipX = flipX;
    spr->draw(core, mat);
}

AABB EnemyCrawler::getBB() const
{
    return AABB(-3.0, 3.0, -4.0, 3.0)+pos;
}

Coord EnemyCrawler::getPos() const
{
    return pos;
}

Coord EnemyCrawler::getVel() const
{
    return vel;
}

void EnemyCrawler::setPos(const Coord& in)
{
    pos = in;
}

void EnemyCrawler::setVel(const Coord& in)
{
    vel = in;
}

void EnemyCrawler::collide(Entity& ent)
{
    if (ent.isPlayerAttack())
    {
        die();
    }
}

bool EnemyCrawler::isDead() const
{
    return dead;
}

bool EnemyCrawler::isEnemy() const
{
    return true;
}

void EnemyCrawler::die()
{
    Game& core = Game::getInstance();
    dead = true;
    core.addEntity(new SmokeEffect(getPos()));
}
