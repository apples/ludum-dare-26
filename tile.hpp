#ifndef TILE_H
#define TILE_H

#include "inugami/spritesheet.hpp"

class Tile
{
public:
    static Tile fromBG();

    Tile();
    Tile(const Tile& in);
    Tile(unsigned char c);

    void draw() const;

    bool isSolid() const;
    bool isDeadly() const;
    bool isBreakable() const;
    bool isWinner() const;

    unsigned char val;

private:
    static Inugami::Spritesheet& getSheet();
};

#endif // TILE_H
