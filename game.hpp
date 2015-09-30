#ifndef CUSTOMCORE_H
#define CUSTOMCORE_H

#include "player.hpp"
#include "level.hpp"
#include "screen.hpp"
#include "font.hpp"

#include "inugami/core.hpp"
#include "inugami/camera.hpp"

#include <map>
#include <string>
#include <vector>
#include <random>

//#include <audiere.h>

class Game : public Inugami::Core
{
    Game() = delete;
    Game(const Game&) = delete;
    Game(const RenderParams &params);
public:
    static Game& getInstance();

    virtual ~Game();

    void frameBeast();
    void tick();
    void draw();

    void tickMainMenu();
    void drawMainMenu();
    void tickGame();
    void drawGame();
    void tickCampaign();
    void drawCampaign();
    void tickEditor();
    void drawEditor();
    void tickEdit();
    void drawEdit();
    void tickEditPicker();
    void drawEditPicker();
    void tickEditLoad();
    void drawEditLoad();
    void tickEditSave();
    void drawEditSave();
    void tickWin();
    void drawWin();

    void addEntity(Entity* e);

    void nextCampaign();

    void loadLevel();
    void loadLevel(const std::string& in);
    void loadLevelNoSpawn(const std::string& in);
    void reloadLevel();

//    void playSound(const std::string& name);

    void start();

    Player* player;
    Level* level;

    std::mt19937 rng;

    std::map<std::string,Inugami::Interface::Proxy> controls;

private:
    void resetState();
    void resetLevel();
    void resetEntities();
    void spawnEnemies();

    void pushScreen(const std::string& name);
    void popScreen();

    void loadCampaign();
    void loadControls();

    Inugami::Camera cam;
    Font font;

    std::vector<Entity*> entities;

    std::map<std::string,Screen> screenMap;
    std::vector<Screen> screenStack;

    bool editmode, dragSelecting, editClickLock;
    float editSX, editSY;
    int editDragX, editDragY, editDragDX, editDragDY;
    Tile selectedTile;
//
//    audiere::AudioDevicePtr audioDevice;
//    std::map<std::string,audiere::SoundEffectPtr> sounds;
//    bool enableSound;

    std::string textBuffer;

    int deathTimer;
    bool playerDead, playerWin, transition;

    std::vector<std::string> campaignLevels;
    int campaignProgress;
};

#endif // CUSTOMCORE_H
