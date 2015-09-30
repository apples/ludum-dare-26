#include "tile.hpp"
#include "imagebank.hpp"

#include "inugami/image.hpp"

#include <algorithm>
#include <vector>

using namespace Inugami;

Tile Tile::fromBG() //static
{
    static const Tile t(7);
    return t;
}

Spritesheet& Tile::getSheet() //static
{
    return ImageBank::getInstance().getSheet("tiles");
}

Tile::Tile()
    : val(0)
{}

Tile::Tile(const Tile& in)
    : val(in.val)
{}

Tile::Tile(unsigned char c)
    : val(c)
{}

void Tile::draw() const
{
    getSheet().draw(val/16, val%16);
}

bool Tile::isSolid() const
{
    static std::vector<char> stiles = {1,2,3,4,5,6};
    return (std::find(stiles.begin(),stiles.end(),val) != stiles.end());
}

bool Tile::isDeadly() const
{
    static std::vector<char> stiles = {16};
    return (std::find(stiles.begin(),stiles.end(),val) != stiles.end());
}

bool Tile::isBreakable() const
{
    static std::vector<char> stiles = {2,5,6};
    return (std::find(stiles.begin(),stiles.end(),val) != stiles.end());
}

bool Tile::isWinner() const
{
    static std::vector<char> stiles = {8};
    return (std::find(stiles.begin(),stiles.end(),val) != stiles.end());
}
