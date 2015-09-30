#include "imagebank.hpp"

#include "inugami/image.hpp"

using namespace Inugami;
using namespace std;

ImageBank& ImageBank::getInstance() //static
{
    static ImageBank bank;
    return bank;
}

ImageBank::ImageBank()
    : sheets()
{
    textures["player"] = new Texture(Image::fromPNG("data/leonard.png"));
    sheets["player"] = new Spritesheet(*textures["player"],8,8);

    textures["tiles"] = new Texture(Image::fromPNG("data/tiles.png"));
    sheets["tiles"] = new Spritesheet(*textures["tiles"],8,8,0,0);

    textures["font"] = new Texture(Image::fromPNG("data/font.png"),true,true);
    sheets["font"] = new Spritesheet(*textures["font"],64,64,0,0);

    textures["effects"] = new Texture(Image::fromPNG("data/effects.png"));
    sheets["effects"] = new Spritesheet(*textures["effects"],8,8);

    textures["enemies"] = new Texture(Image::fromPNG("data/enemies.png"));
    sheets["enemies"] = new Spritesheet(*textures["enemies"],8,8);

    textures["menu"] = new Texture(Image::fromPNG("data/menu.png"));
    sheets["menu"] = new Spritesheet(*textures["menu"],160,120);
}

ImageBank::~ImageBank()
{
    for (auto& p : textures) delete p.second;
    for (auto& p : sheets) delete p.second;
}

Texture& ImageBank::getTexture(const string& name)
{
    return *textures[name];
}

Spritesheet& ImageBank::getSheet(const string& name)
{
    return *sheets[name];
}
