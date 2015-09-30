#include "game.hpp"

#include "meta.hpp"
#include "font.hpp"
#include "imagebank.hpp"
#include "enemies.hpp"
#include "entitytimer.hpp"

#include "inugami/camera.hpp"
#include "inugami/geometry.hpp"
#include "inugami/image.hpp"
#include "inugami/interface.hpp"
#include "inugami/loaders.hpp"
#include "inugami/math.hpp"
#include "inugami/shader.hpp"
#include "inugami/shaderprogram.hpp"
#include "inugami/transform.hpp"
#include "inugami/utility.hpp"

#include <algorithm>
#include <fstream>
#include <utility>
#include <functional>
#include <string>
#include <sstream>

using namespace std;
using namespace Inugami;

static Game::RenderParams makeParams() //static
{
    Game::RenderParams rval;
    rval.width = 800;
    rval.height = 600;
    rval.fullscreen = false;
    rval.vsync = false;
    rval.fsaaSamples = 2;
    return rval;
}

Game& Game::getInstance() //static
{
    static Game instance(makeParams());
    return instance;
}

Game::Game(const RenderParams &params)
    : Core(params)
    , player(nullptr)
    , level(nullptr)
    , rng()
    , controls()

    , cam()
    , font(ImageBank::getInstance().getTexture("font"),64,64)
    , entities()

    , screenMap()
    , screenStack()

    , editmode(false)
    , dragSelecting(false)
    , editClickLock(false)
    , editSX(1.f)
    , editSY(1.f)
    , editDragX(0)
    , editDragY(0)
    , editDragDX(0)
    , editDragDY(0)
    , selectedTile(1)

//    , audioDevice(OpenDevice(""))
//    , sounds()
//    , enableSound(true)

    , deathTimer(0)
    , playerDead(false)
    , playerWin(false)
    , transition(false)

    , campaignLevels()
    , campaignProgress(0)
{
    addCallback(std::bind(&Game::frameBeast, this), 60.0);

    cam.ortho(-80.f, 80.f, -60.f, 60.f, -10.f, 10.f);

    setWindowTitle("Tower", true);

    Screen scr;

    scr.draw = bind(&Game::drawMainMenu, this);
    scr.tick = bind(&Game::tickMainMenu, this);
    scr.enter = []{};
    scr.leave = []{};
    scr.opaque = true;
    scr.freeze = true;
    screenMap["mainmenu"] = scr;

    scr.draw = bind(&Game::drawGame, this);
    scr.tick = bind(&Game::tickGame, this);
    scr.enter = []{};
    scr.leave = []{};
    scr.opaque = true;
    scr.freeze = true;
    screenMap["game"] = scr;

    scr.draw = bind(&Game::drawCampaign, this);
    scr.tick = bind(&Game::tickCampaign, this);
    scr.enter = []{};
    scr.leave = [&]{popScreen();};
    scr.opaque = false;
    scr.freeze = false;
    screenMap["campaign"] = scr;

    scr.draw = bind(&Game::drawEditor, this);
    scr.tick = bind(&Game::tickEditor, this);
    scr.enter = []{};
    scr.leave = [&]{popScreen();};
    scr.opaque = false;
    scr.freeze = false;
    screenMap["editor"] = scr;

    scr.draw = bind(&Game::drawEdit, this);
    scr.tick = bind(&Game::tickEdit, this);
    scr.enter = bind(&Game::resetLevel, this);
    scr.leave = [&]{level->set(); spawnEnemies();};
    scr.opaque = false;
    scr.freeze = true;
    screenMap["edit"] = scr;

    scr.draw = bind(&Game::drawEditPicker, this);
    scr.tick = bind(&Game::tickEditPicker, this);
    scr.enter = []{};
    scr.leave = []{};
    scr.opaque = true;
    scr.freeze = true;
    screenMap["edit.pick"] = scr;

    scr.draw = bind(&Game::drawEditLoad, this);
    scr.tick = bind(&Game::tickEditLoad, this);
    scr.enter = [&]{iface->getBuffer();};
    scr.leave = []{};
    scr.opaque = true;
    scr.freeze = true;
    screenMap["edit.load"] = scr;

    scr.draw = bind(&Game::drawEditSave, this);
    scr.tick = bind(&Game::tickEditSave, this);
    scr.enter = [&]{iface->getBuffer();};
    scr.leave = []{};
    scr.opaque = true;
    scr.freeze = true;
    screenMap["edit.save"] = scr;

    scr.draw = bind(&Game::drawWin, this);
    scr.tick = bind(&Game::tickWin, this);
    scr.enter = []{};
    scr.leave = []{};
    scr.opaque = true;
    scr.freeze = true;
    screenMap["win"] = scr;

    pushScreen("mainmenu");

//    if (!audioDevice)
//    {
//        logger->log<5>("Failed to open audio device.");
//        enableSound = false;
//    }
//
//    if (enableSound)
//    {
//        auto loadSound = [&](const string& name, const string& file, SoundEffectType t)
//        {
//            SoundEffectPtr sound = OpenSoundEffect(audioDevice, file.c_str(), t);
//            if (!sound) logger->log<5>("Failed to load "+file+".");
//            else sounds[name] = sound;
//        };
//
//        loadSound("test", "data/test.wav", MULTIPLE);
//    }

    loadCampaign();
    loadControls();
}

Game::~Game()
{
    for (Entity* e : entities) delete e;
    delete level;
}

void Game::frameBeast()
{
    ScopedProfile prof(profiler, "Game");
    tick();
    draw();
}

void Game::tick()
{
    ScopedProfile prof(profiler, "Tick");

    auto keyESC = iface->getProxy(0_ivkFunc);

    iface->poll();

    if (keyESC.pressed()) popScreen();

    if (shouldClose() || screenStack.empty())
    {
        running = false;
        return;
    }

    int iter = screenStack.size()-1;
    do screenStack[iter].tick();
    while (!screenStack[iter].freeze && --iter > 0);
}

void Game::draw()
{
    ScopedProfile prof(profiler, "Draw");

    beginFrame();

    int iter = screenStack.size();
    while (iter>0) if (screenStack[--iter].opaque) break;
    while (iter<screenStack.size()) screenStack[iter++].draw();

    endFrame();
}

void Game::tickMainMenu()
{
    ScopedProfile prof(profiler, "Tick Main Menu");

    const int mouseX  = (iface->getMousePos().x-400.0)/400.0*80.0;
    const int mouseY  = (300.0-iface->getMousePos().y)/300.0*60.0;

    const AABB button1(-40,40,-17,3);
    const AABB button2(-40,40,-44,-24);
    const AABB cursor(mouseX,mouseX,mouseY,mouseY);

    if (iface->mousePressed('L'_ivm))
    {
        if (cursor.intersects(button1))
        {
            campaignProgress = 0;
            loadLevel(campaignLevels[campaignProgress]);
            pushScreen("game");
            pushScreen("campaign");
            return;
        }

        if (cursor.intersects(button2))
        {
            campaignProgress = 0;
            loadLevel();
            pushScreen("game");
            pushScreen("editor");
            return;
        }
    }
}

void Game::drawMainMenu()
{
    ScopedProfile prof(profiler, "Draw Main Menu");

    applyCam(cam);
    Transform mat;
    mat.translate(Vec3{48.f,-68.f,0.f});
    modelMatrix(mat);

    ImageBank::getInstance().getTexture("menu").bind(0);
    Mesh(Geometry::fromRect(256,256)).draw();
}

void Game::tickGame()
{
    ScopedProfile prof(profiler, "Tick Game");

    for (int i=0; i<entities.size(); ++i)
    {
        if (entities[i]->isDead())
        {
            if (entities[i]!=player)
            {
                delete entities[i];
            }
            entities[i] = nullptr;
        }
        else
        {
            entities[i]->tick();
            for (int j=i; j<entities.size(); ++j)
            {
                if (entities[i]->getBB().intersects(entities[j]->getBB()))
                {
                    entities[i]->collide(*entities[j]);
                    entities[j]->collide(*entities[i]);
                }
            }
        }
    }
    entities.erase(remove(entities.begin(), entities.end(), nullptr), entities.end());

    if (!playerWin && !player->isDead()) for (auto& p : level->getCollisions(player->getBB()))
    {
        if (p.first->isWinner())
        {
            playerWin = true;
            player->die();
        }
    }
}

void Game::drawGame()
{
    ScopedProfile prof(profiler, "Draw Game");

    Camera playerCam;
    playerCam.ortho(-80.f*editSX, 80.f*editSY, -60.f*editSX, 60.f*editSY, -10.f, 10.f);
    playerCam.translate(Vec3{-int(player->getPos().x),-int(player->getPos().y+30),0.f});
    applyCam(playerCam);

    level->draw();
    for (Entity* e : entities) e->draw();
}

void Game::tickCampaign()
{
    ScopedProfile prof(profiler, "Tick Campaign");

    if (!transition && player->isDead())
    {
        transition = true;
        if (playerWin)
        {
            addEntity(new EntityTimer(60, bind(&Game::nextCampaign, this)));
        }
        else
        {
            addEntity(new EntityTimer(60, bind(&Game::reloadLevel, this)));
        }
    }
}

void Game::drawCampaign()
{
    ScopedProfile prof(profiler, "Draw Campaign");
}

void Game::tickEditor()
{
    ScopedProfile prof(profiler, "Tick Editor");

    auto keyF9 = iface->getProxy(9_ivkFunc);

    if (keyF9.pressed())
    {
        pushScreen("edit");
    }

    if (!transition && player->isDead())
    {
        transition = true;
        addEntity(new EntityTimer(60, bind(&Game::reloadLevel, this)));
    }
}

void Game::drawEditor()
{
    ScopedProfile prof(profiler, "Draw Editor");

    applyCam(cam);

    Transform mat;

    mat.translate(Vec3{-76, -56, 0});
    mat.scale(Vec3{0.5f, 0.5f, 1.f});
    font.drawString("[F9] Edit",mat);
}

void Game::tickEdit()
{
    ScopedProfile prof(profiler, "Tick Edit");

    auto keyUp     = iface->getProxy('W'_ivk);
    auto keyDown   = iface->getProxy('S'_ivk);
    auto keyLeft   = iface->getProxy('A'_ivk);
    auto keyRight  = iface->getProxy('D'_ivk);
    auto keyUp2    = iface->getProxy('U'_ivkArrow);
    auto keyDown2  = iface->getProxy('D'_ivkArrow);
    auto keyLeft2  = iface->getProxy('L'_ivkArrow);
    auto keyRight2 = iface->getProxy('R'_ivkArrow);
    auto keyLShift = iface->getProxy('L'_ivkShift);
    auto keyF1     = iface->getProxy(1_ivkFunc);
    auto keyF2     = iface->getProxy(2_ivkFunc);
    auto keyF5     = iface->getProxy(5_ivkFunc);
    auto keyF6     = iface->getProxy(6_ivkFunc);
    auto keyF9     = iface->getProxy(9_ivkFunc);

    if (keyF1.pressed())
    {
        pushScreen("edit.pick");
        return;
    }

    if (keyF2.pressed())
    {
        level->setSpawn(player->getPos().x/8.0, player->getPos().y/8.0);
        return;
    }

    if (keyF5.pressed())
    {
        pushScreen("edit.load");
        return;
    }

    if (keyF6.pressed())
    {
        pushScreen("edit.save");
        return;
    }

    if (keyF9.pressed())
    {
        popScreen();
        return;
    }

    if (keyLShift)
    {
        int num = iface->getMouseWheel().y;
        if (num>0) for (int i=0; i<num; ++i) editSX*=0.9f, editSY*=0.9f;
        else if (num<0) for (int i=0; i>num; --i) editSX*=1.1f, editSY*=1.1f;
        iface->setMouseWheel(0,0);
    }
    else
    {
        selectedTile = int(double(selectedTile.val)+iface->getMouseWheel().y)%256;
        iface->setMouseWheel(0,0);
    }

    Coord ppos = player->getPos();
    if (keyLeft  || keyLeft2)  ppos.x -= 1;
    if (keyRight || keyRight2) ppos.x += 1;
    if (keyDown  || keyDown2)  ppos.y -= 1;
    if (keyUp    || keyUp2)    ppos.y += 1;
    player->setPos(ppos);

    const int mouseX  = (iface->getMousePos().x-400.0)/400.0*80.0;
    const int mouseY  = (300.0-iface->getMousePos().y)/300.0*60.0;
    const int mouseTX = (mouseX*editSX+player->getPos().x)/8.0;
    const int mouseTY = (mouseY*editSY+player->getPos().y+30)/8.0;

    if (iface->mousePressed('L'_ivm) && mouseX>=-76&&mouseX<-68&&mouseY>=48&&mouseY<56 && !editClickLock)
    {
        pushScreen("edit.pick");
        return;
    }
    else if (iface->mouseState('L'_ivm) && !editClickLock)
    {
        level->setTile(mouseTX, mouseTY, selectedTile);
    }

    if (iface->mouseState('R'_ivm) && !dragSelecting)
    {
        dragSelecting = true;
        editDragX = mouseTX;
        editDragY = mouseTY;
    }

    if (dragSelecting)
    {
        editDragDX = mouseTX;
        editDragDY = mouseTY;
    }

    if (!iface->mouseState('R'_ivm) && dragSelecting)
    {
        dragSelecting = false;

        if (editDragX>editDragDX) std::swap(editDragX, editDragDX);
        if (editDragY>editDragDY) std::swap(editDragY, editDragDY);

        for (int i=editDragX; i<=editDragDX; ++i)
        {
            for (int j=editDragY; j<=editDragDY; ++j)
            {
                level->setTile(i, j, selectedTile);
            }
        }
    }

    if (iface->mousePressed('M'_ivm))
    {
        selectedTile = level->tileAt(mouseTX, mouseTY);
    }

    if (!iface->mouseState('L'_ivm)) editClickLock = false;
}

void Game::drawEdit()
{
    ScopedProfile prof(profiler, "Draw Edit");

    Camera playerCam;
    playerCam.ortho(-80.f*editSX, 80.f*editSY, -60.f*editSX, 60.f*editSY, -10.f, 10.f);
    playerCam.translate(Vec3{-int(player->getPos().x),-int(player->getPos().y+30),0.f});
    applyCam(playerCam);

    Transform mat;

    if (dragSelecting)
    {
        int edx  =  editDragX*8.f+(editDragDX<editDragX?8.f:0.f);
        int edy  =  editDragY*8.f+(editDragDY<editDragY?8.f:0.f);
        int eddx = editDragDX*8.f-(editDragDX<editDragX?8.f:0.f);
        int eddy = editDragDY*8.f-(editDragDY<editDragY?8.f:0.f);

        mat.reset();
        mat.translate(Vec3{edx,edy,0.f});
        modelMatrix(mat);

        Texture selectTex(Image(1,1,{{85,85,256,64}}));
        selectTex.bind(0);

        Mesh(Geometry::fromRect(eddx-edx+8.f,eddy-edy+8.f,1.f,1.f)).draw();
    }

    mat.reset();
    mat.translate(Vec3{level->spawnX*8+4,level->spawnY*8+4,0.f});
    modelMatrix(mat);

    ImageBank::getInstance().getSheet("player").draw(7,7);

    applyCam(cam);

    mat.reset();
    mat.translate(Vec3{-72,52,0});
    modelMatrix(mat);

    Texture derp(Image(1,1,{{85,85,85,255}}));
    derp.bind(0);

    Mesh(Geometry::fromRect(12,12)).draw();

    mat.reset();
    mat.translate(Vec3{-76,48,0});
    modelMatrix(mat);

    Tile(selectedTile).draw();

    stringstream ss;
    ss << "(" << selectedTile.val/16 << "," << selectedTile.val%16 << ")";
    string s = ss.str();

    mat.translate(Vec3{10, 6, 0});
    mat.scale(Vec3{0.5f, 0.5f, 1.f});
    font.drawString(s,mat);

    mat.reset();
    mat.translate(Vec3{-76, -44, 0});
    mat.scale(Vec3{0.5f, 0.5f, 1.f});
    font.drawString("[F1] Pick Tile\n[F2] Set Spawn\n[F5] Load\n[F6] Save",mat);
}

void Game::tickEditPicker()
{
    ScopedProfile prof(profiler, "Tick Tile Picker");

    auto keyF1 = iface->getProxy(1_ivkFunc);

    if (keyF1.pressed())
    {
        popScreen();
    }

    const int mouseX  = (iface->getMousePos().x-400.0)/400.0*80.0;
    const int mouseY  = (300.0-iface->getMousePos().y)/300.0*60.0;

    static constexpr int gridw = 18;

    if (iface->mousePressed('L'_ivm))
    {
        int r = (60-mouseY)/8;
        int c = (mouseX+gridw*4)/8;
        int t = r*gridw+c;

        if (t<256)
        {
            selectedTile = t;
            popScreen();
            editClickLock = true;
            return;
        }
    }
}

void Game::drawEditPicker()
{
    ScopedProfile prof(profiler, "Draw Tile Picker");

    applyCam(cam);

    Transform mat;

    static constexpr int gridw = 18;

    for (int i=0; i<256; ++i)
    {
        mat.push();
        mat.translate(Vec3{-gridw*4+(i%gridw)*8,52-(i/gridw)*8,0});
        modelMatrix(mat);
        mat.pop();
        Tile(i).draw();
    }
}

void Game::tickEditLoad()
{
    ScopedProfile prof(profiler, "Tick Edit Load");

    auto keyEnter = iface->getProxy('\n'_ivk);
    auto keyBack  = iface->getProxy('\b'_ivk);

    textBuffer += iface->getBuffer();

    if (keyEnter.pressed())
    {
        loadLevelNoSpawn("data/levels/"+textBuffer+".lvl");
        popScreen();
        popScreen();
    }

    if (keyBack.pressed())
    {
        textBuffer = textBuffer.substr(0,textBuffer.length()-1);
    }
}

void Game::drawEditLoad()
{
    ScopedProfile prof(profiler, "Draw Edit Load");

    string text = "Load:"+textBuffer;

    applyCam(cam);

    Transform mat;

    mat.scale(Vec3{0.5f, 0.5f, 1.f});
    mat.translate(Vec3{-4*int(text.size()), -4, 0});
    font.drawString(text, mat);
}

void Game::tickEditSave()
{
    ScopedProfile prof(profiler, "Tick Edit Save");

    auto keyEnter = iface->getProxy('\n'_ivk);
    auto keyBack  = iface->getProxy('\b'_ivk);

    textBuffer += iface->getBuffer();

    if (keyEnter.pressed())
    {
        level->set();
        level->toFile("data/levels/"+textBuffer+".lvl");
        popScreen();
    }

    if (keyBack.pressed())
    {
        textBuffer = textBuffer.substr(0,textBuffer.length()-1);
    }
}

void Game::drawEditSave()
{
    ScopedProfile prof(profiler, "Draw Edit Save");

    string text = "Save:"+textBuffer;

    applyCam(cam);

    Transform mat;

    mat.scale(Vec3{0.5f, 0.5f, 1.f});
    mat.translate(Vec3{-4*int(text.size()), -4, 0});
    font.drawString(text, mat);
}

void Game::tickWin()
{
    ScopedProfile prof(profiler, "Tick Edit Save");
}

void Game::drawWin()
{
    ScopedProfile prof(profiler, "Draw Edit Save");

    applyCam(cam);

    Transform mat;
    mat.translate(Vec3{-4*14, -12, 0});
    font.drawString("YOU ARE WINNER\n  PRESS  ESC  ", mat);
}

void Game::addEntity(Entity* e)
{
    if (!e->isDead()) entities.push_back(e);
}

void Game::nextCampaign()
{
    ++campaignProgress;
    if (campaignProgress<campaignLevels.size())
    {
        loadLevel(campaignLevels[campaignProgress]);
    }
    else
    {
        screenStack.clear();
        pushScreen("mainmenu");
        pushScreen("win");
    }
}

void Game::loadLevel()
{
    resetState();

    delete level;
    level = new Level(Level::fromDefault());

    resetEntities();
    spawnEnemies();
}

void Game::loadLevel(const std::string& in)
{
    resetState();

    delete level;
    level = new Level(Level::fromFile(in));

    resetEntities();
    spawnEnemies();
}

void Game::loadLevelNoSpawn(const std::string& in)
{
    resetState();

    delete level;
    level = new Level(Level::fromFile(in));

    resetEntities();
}

void Game::reloadLevel()
{
    resetState();
    resetLevel();
    resetEntities();
    spawnEnemies();
}

//void Game::playSound(const std::string& name)
//{
//    if (!enableSound) return;
//    auto iter = sounds.find(name);
//    if (iter != sounds.end()) iter->second->play();
//}

void Game::start()
{
    go();
}

void Game::resetState()
{
    transition = false;

    editSX = 1.f;
    editSY = 1.f;

    playerWin = false;
    playerDead = false;
}

void Game::resetLevel()
{
    level->reset();

    for (int i=0; i<entities.size(); ++i)
    {
        if (entities[i] != player)
        {
            delete entities[i];
            entities[i] = nullptr;
        }
    }
    entities.erase(remove(entities.begin(), entities.end(), nullptr), entities.end());
}

void Game::resetEntities()
{
    for (Entity* e : entities) delete e;
    entities.clear();

    player = new Player;
    player->setPos(Coord(level->spawnX*8.0+4.0,level->spawnY*8.0+4.0));
    addEntity(player);
}

void Game::spawnEnemies()
{
    for (int x=0; x<level->width; ++x)
    {
        for (int y=0; y<level->height; ++y)
        {
            switch (int(level->tileAt(x,y).val) - 15*16)
            {
                case 0:
                {
                    level->setTile(x,y,0);
                    Entity* e = new EnemyCrawler;
                    e->setPos(Coord(x*8+4,y*8+4));
                    addEntity(e);
                break;}

                case 1:
                {
                    level->setTile(x,y,0);
                    Entity* e = new EnemyKnight;
                    e->setPos(Coord(x*8+4,y*8+4));
                    addEntity(e);
                break;}
            }
        }
    }
}

void Game::pushScreen(const string& name)
{
    screenStack.push_back(screenMap[name]);
    screenStack.back().enter();
}

void Game::popScreen()
{
    auto scr = screenStack.back();
    screenStack.pop_back();
    scr.leave();
}

void Game::loadCampaign()
{
    string tmp;
    ifstream file("data/levels/campaign.txt");
    while (getline(file, tmp))
    {
        campaignLevels.push_back("data/levels/"+tmp+".lvl");
    }
}

void Game::loadControls()
{
    string tmp;
    ifstream file("data/controls.txt");
    while (getline(file, tmp))
    {
        auto eqPos = tmp.find('=');
        if (eqPos == string::npos) continue;
        string name = tmp.substr(0,eqPos);
        string val  = tmp.substr(eqPos+1);
        if (name == "" || val == "") continue;
        int vk = -1;
        if (val.size() == 1)
        {
            vk = operator"" _ivk(val[0]);
        }
        else
        {
            transform(val.begin(), val.end(), val.begin(), ::tolower);
            if (val == "left") vk = 'L'_ivkArrow;
            else if (val == "right") vk = 'R'_ivkArrow;
            else if (val == "up") vk = 'U'_ivkArrow;
            else if (val == "down") vk = 'D'_ivkArrow;
        }
        if (vk == -1) continue;
        controls[name] = iface->getProxy(vk);
    }
}
