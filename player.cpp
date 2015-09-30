#include "player.hpp"

#include "game.hpp"
#include "playermelee.hpp"
#include "playerranged.hpp"
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

Player::Player()
    : pos()
    , vel()
    , grounded(false)
    , jumpReady(false)
    , flipX(false)
    , gliding(false)
    , wallSlide(0)
    , attackTimer(0)
    , curSprite("idle")
    , sprites()
    , keys()
    , dieing(false)
    , dead(false)
{
    Game& core = Game::getInstance();

    Spritesheet& sheet8x8 = ImageBank::getInstance().getSheet("player");
    AnimatedSprite* sprite = nullptr;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{0,0}});
    sprite->setSequence({{0,1}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["idle"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{1,0},{1,1}});
    sprite->setSequence({{0,12},{1,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["walk"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{3,0}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["jump.rise"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{3,1}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["jump.peak"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{3,2},{3,3},{3,4}});
    sprite->setSequence({{0,6},{1,6},{2,6}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["jump.fall"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{2,0}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["attack"] = sprite;

    sprite = new AnimatedSprite(sheet8x8);
    sprite->setSprites({{3,5}});
    sprite->setSequence({{0,12}});
    sprite->setMode(AnimatedSprite::Mode::LOOP);
    sprites["slide"] = sprite;

    keys = core.controls;
}

Player::~Player()
{
    for (auto& p : sprites) delete p.second;
}

void Player::tick()
{
    Game& core = Game::getInstance();

    static constexpr double gravity = 0.15;
    static constexpr double glide = 0.05;
    static constexpr double friction = 0.5;
    static constexpr double kick = 3.0;
    static constexpr double walkacc = 1.0;
    static constexpr double airaccel = 0.1;
    static constexpr double airfriction = 0.9;
    static constexpr double jumpstr = 2.5;
    static constexpr double maxFallSpeed = 2.5;
    static constexpr double walljumpstr = 3.0;

    curSprite = "idle";

    if (keys["left"] && !keys["right"])
    {
        vel.x -= (grounded?walkacc:airaccel);
        curSprite = "walk";
        flipX = true;
    }
    else if (keys["right"] && !keys["left"])
    {
        vel.x += (grounded?walkacc:airaccel);
        curSprite = "walk";
        flipX = false;
    }
    else
    {
        if (grounded) vel.x = 0;
    }

    if (keys["jump"].pressed())
    {
        if (wallSlide && vel.y <= 0.0)
        {
            vel.y = jumpstr;
            vel.x += wallSlide*walljumpstr;
        }
        else if (grounded)
        {
            vel.y = jumpstr;
        }
    }

    if (grounded) vel.x *= friction;
    else vel.x *= airfriction;

    AABB current = getBB();
    AABB target;

    grounded = false;
    gliding = false;

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

            if ((ac.isSolid() && !an.isSolid()) || (bc.isSolid() && !bn.isSolid()))
            {
                vel.x = icleft*8.0-current.left;
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

            if ((ac.isSolid() && !an.isSolid()) || (bc.isSolid() && !bn.isSolid()))
            {
                vel.x = (icright+1)*8.0-current.right;
                wallSlide = -1;
            }
        }
    }

    if (grounded) wallSlide = 0;

    if (vel.y<=0.0&&keys["jump"]) gliding = true;

    if (!grounded)
    {
        if (vel.y > 1.0) curSprite = "jump.rise";
        else if ((vel.y<-1.0&&!gliding)||vel.y==-jumpstr) curSprite = "jump.fall";
        else curSprite = "jump.peak";
    }

    pos += vel;

    for (auto& p : core.level->getCollisions(getBB())) if (p.first->isDeadly()) {die(); break;}

    vel.y -= (gliding)? glide : gravity;
    if (vel.y < -maxFallSpeed) vel.y = -maxFallSpeed;

    if (keys["melee"].pressed())
    {
        attackTimer = 3;

        Coord apos(0,0);
        int state = 0;

//        if (keys["up"])
//        {
//            apos.y = 8;
//            state = 1;
//            if (keys["left"]||keys["right"])
//            {
//                apos.x = 6;
//                state = 2;
//            }
//        }
//        else if (keys["down"])
//        {
//            apos.y = -8;
//            state = 3;
//            if (keys["left"]||keys["right"])
//            {
//                apos.x = 6;
//                state = 4;
//            }
//        }
//        else
        {
            apos.x = 8;
        }

        Coord avel = getVel()*(apos/8);

        if (flipX) apos.x *= -1;

        core.addEntity(new PlayerMelee(getPos()+apos, avel, state, flipX));
    }

    if (keys["shoot"].pressed())
    {
        attackTimer = 3;

        Coord apos(0,0);

//        if (keys["up"])
//        {
//            apos.y = 4;
//            if (keys["left"]||keys["right"]||wallSlide)
//            {
//                apos.x = 3;
//            }
//        }
//        else if (keys["down"])
//        {
//            apos.y = -4;
//            if (keys["left"]||keys["right"]||wallSlide)
//            {
//                apos.x = 3;
//            }
//        }
//        else
        {
            apos.x = 4;
        }

        Coord avel = getVel()*(apos/4);

        if (flipX) apos.x *= -1;
        if (wallSlide) apos.x *= -1;

        avel += apos/2;

        core.addEntity(new PlayerRanged(getPos()+apos, avel, flipX));
    }

    if (attackTimer>0) --attackTimer, curSprite = "attack";
    if (wallSlide != 0 && vel.y <= 0.0)
    {
        vel.y*=friction;
        curSprite = "slide";
        flipX = (wallSlide > 0);
    }
}

void Player::draw()
{
    Game& core = Game::getInstance();

    Transform mat;
    mat.translate(Vec3{int(pos.x), int(pos.y), 0.f});
    core.modelMatrix(mat);

    auto spr = sprites[curSprite];
    spr->flipX = flipX;
    spr->draw(core, mat);
}

AABB Player::getBB() const
{
    return AABB(-3.0, 3.0, -4.0, 4.0)+pos;
}

Coord Player::getPos() const
{
    return pos;
}

Coord Player::getVel() const
{
    return vel;
}

void Player::setPos(const Coord& in)
{
    pos = in;
}

void Player::setVel(const Coord& in)
{
    vel = in;
}

void Player::collide(Entity& ent)
{
    if (ent.isEnemy()
     || ent.isEnemyAttack())
    {
        die();
    }
}

bool Player::isDead() const
{
    return dead;
}

void Player::die()
{
    Game& core = Game::getInstance();
    dead = true;
    core.addEntity(new SmokeEffect(getPos()));
}
