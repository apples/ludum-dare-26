#ifndef LEVEL_H
#define LEVEL_H

#include "aabb.hpp"
#include "tile.hpp"

#include "inugami/geometry.hpp"
#include "inugami/mesh.hpp"
#include "inugami/utility.hpp"

#include <utility>
#include <vector>

using namespace Inugami;

class Level
{
public:
    static Level fromFile(const std::string& filename);
    static Level fromDefault();

    Level();
    Level(const Level& in);
    Level(Level&& in);
    ~Level();

    void toFile(const std::string& filename) const;

    void draw();

    const Tile& tileAt(int x, int y) const;

    void setSpawn(int x, int y);
    void setTile(int x, int y, Tile t);

    std::vector<std::pair<const Tile*,AABB>> getCollisions(const AABB& in) const;

    void set();
    void reset();

    ConstAttr<int,Level> width, height;
    ConstAttr<int,Level> spawnX, spawnY;

private:
    static constexpr int chunkSize = 64;

    void verifyMeshes();

    std::vector<Tile> tiles;
    std::vector<Tile> original;
    Tile nullTile;

    std::vector<Mesh*> meshesBG;
    std::vector<Mesh*> meshesFG;
};

#endif // LEVEL_H
