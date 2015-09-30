#include "level.hpp"

#include "game.hpp"
#include "meta.hpp"
#include "imagebank.hpp"

#include "inugami/transform.hpp"

#include <algorithm>
#include <fstream>
#include <string>
#include <utility>

using namespace std;
using namespace Inugami;

Level Level::fromFile(const string& filename) //static
{
    Level rval;

    ifstream file(filename.c_str(), ios::binary);
    vector<char> tmp(4,'X');
    int tmpi;

    //Sentry
    file.read(&tmp[0], 1);
    if (tmp[0] != char(130)) throw GameError("Invalid LVL file.");

    //Format
    file.read(&tmp[0], 3);
    if (tmp[0]!='L'||tmp[1]!='V'||tmp[2]!='L') throw GameError("Invalid LVL file.");

    //Corruption check
    file.read(&tmp[0], 2);
    if (tmp[0]!=13||tmp[1]!=10) throw GameError("Corrupt LVL file.");
    file.read(&tmp[0], 1);
    if (tmp[0]!=10) throw GameError("Corrupt LVL file.");

    //Spawn point
    tmpi=0;
    file.read(&tmp[0], 4);
    for (int i=0; i<4; ++i) tmpi += tmp[i]<<(i*8);
    rval.spawnX = tmpi;

    tmpi=0;
    file.read(&tmp[0], 4);
    for (int i=0; i<4; ++i) tmpi += tmp[i]<<(i*8);
    rval.spawnY = tmpi;

    //Width
    tmpi=0;
    file.read(&tmp[0], 4);
    for (int i=0; i<4; ++i) tmpi += tmp[i]<<(i*8);
    rval.width = tmpi;

    //Height
    tmpi=0;
    file.read(&tmp[0], 4);
    for (int i=0; i<4; ++i) tmpi += tmp[i]<<(i*8);
    rval.height = tmpi;

    if (rval.width<=0
     || rval.height<=0
     || rval.width>=32768
     || rval.height>=32768)
    {
        throw GameError("Invalid LVL dimensions.");
    }

    unsigned int bytes = rval.width*rval.height;

    rval.tiles.resize(bytes);

    vector<char> data(bytes);
    file.read(&data[0], bytes);
    if (file.gcount() != bytes) throw GameError("Missing LVL data.");

    copy(data.begin(), data.end(), rval.tiles.begin());

    rval.set();

    return rval;
}

Level Level::fromDefault() //static
{
    Level rval;

    rval.width = 32;
    rval.height = 32;
    rval.spawnX = 2;
    rval.spawnY = 2;

    rval.tiles.resize(32*32);

    for (int i=0; i<32*32; ++i) rval.tiles[i] = (i%32==0||i%32==31||i/32==0||i/32==31)? 1 : 0;

    rval.set();

    return rval;
}

Level::Level()
    : width(0)
    , height(0)
    , tiles()
    , original()
    , nullTile(1)
    , meshesBG()
    , meshesFG()
{}

Level::Level(const Level& in)
    : width(in.width)
    , height(in.height)
    , tiles(in.tiles)
    , nullTile(in.nullTile)
    , meshesBG(in.meshesBG.size())
    , meshesFG(in.meshesFG.size())
{
    transform(in.meshesFG.begin(), in.meshesFG.end(), meshesFG.begin(), [](Mesh* m){return new Mesh(*m);});
    transform(in.meshesBG.begin(), in.meshesBG.end(), meshesBG.begin(), [](Mesh* m){return new Mesh(*m);});
}

Level::Level(Level&& in)
    : width(0)
    , height(0)
    , tiles(move(in.tiles))
    , nullTile(in.nullTile)
    , meshesBG(move(in.meshesBG))
    , meshesFG(move(in.meshesFG))
{}

Level::~Level()
{
    for (Mesh* mp : meshesFG) delete mp;
    for (Mesh* mp : meshesBG) delete mp;
}

void Level::toFile(const string& filename) const
{
    ofstream file(filename.c_str(), ios::binary);
    vector<char> tmp(4,'X');
    int tmpi;

    //Sentry
    tmp[0] = 130;
    file.write(&tmp[0], 1);

    //Format
    tmp[0]='L';
    tmp[1]='V';
    tmp[2]='L';
    file.write(&tmp[0], 3);

    //Corruption check
    tmp[0]=13;
    tmp[1]=10;
    file.write(&tmp[0], 2);
    tmp[0]=10;
    file.write(&tmp[0], 1);

    //Spawn point
    tmpi=spawnX;
    for (int i=0; i<4; ++i) tmp[i] = (tmpi>>(i*8))%256;
    file.write(&tmp[0], 4);

    tmpi=spawnY;
    for (int i=0; i<4; ++i) tmp[i] = (tmpi>>(i*8))%256;
    file.write(&tmp[0], 4);

    //Width
    tmpi=width;
    for (int i=0; i<4; ++i) tmp[i] = (tmpi>>(i*8))%256;
    file.write(&tmp[0], 4);

    //Height
    tmpi=height;
    for (int i=0; i<4; ++i) tmp[i] = (tmpi>>(i*8))%256;
    file.write(&tmp[0], 4);

    unsigned int bytes = width*height;

    vector<char> data(bytes);
    transform(original.begin(), original.end(), data.begin(), [](const Tile& t){return t.val;});
    file.write(&data[0], bytes);
}

void Level::draw()
{
    ScopedProfile prof(profiler, "Level::draw()");

    verifyMeshes();

    Transform mat;
    Game::getInstance().modelMatrix(mat);

    ImageBank::getInstance().getTexture("tiles").bind(0);
    for (Mesh* mp : meshesBG) mp->draw();
    for (Mesh* mp : meshesFG) mp->draw();
}

const Tile& Level::tileAt(int x, int y) const
{
    if (x<0 || y<0 || x>=width || y>=height) return nullTile;
    return tiles[x+y*width];
}

void Level::setSpawn(int x, int y)
{
    spawnX = x;
    spawnY = y;
}

void Level::setTile(int x, int y, Tile t)
{
    if (x<0 || y<0 || x>=width || y>=height) return;
    tiles[x+y*width] = t;
    int i = (x+y*width)/chunkSize;
    if (i<meshesFG.size())
    {
        delete meshesFG[i];
        meshesFG[i] = nullptr;
    }
}

vector<pair<const Tile*,AABB>> Level::getCollisions(const AABB& in) const
{
    vector<pair<const Tile*,AABB>> rval;

    for (int x=in.left/8.0; x<int(in.right/8.0)+1; ++x)
    {
        for (int y=in.bottom/8.0; y<width-int(double(width)-in.top/8.0); ++y)
        {
            const Tile* t = &tileAt(x, y);
            rval.push_back(make_pair(t, AABB(x*8,(x+1)*8,y*8,(y+1)*8)));
        }
    }

    return rval;
}

void Level::set()
{
    original = tiles;
}

void Level::reset()
{
    tiles = original;
    for (auto& p : meshesFG) {delete p;}
    meshesFG.clear();
}

void Level::verifyMeshes()
{
    ScopedProfile prof(profiler, "Level::verifyMeshes()");

    const int chunkCount = tiles.size()/chunkSize+1;

    if (meshesBG.size() != chunkCount)
    {
        for (auto& mp : meshesBG) {delete mp; mp = nullptr;}
        meshesBG.resize(chunkCount, nullptr);
    }

    if (meshesFG.size() != chunkCount)
    {
        for (auto& mp : meshesFG) {delete mp; mp = nullptr;}
        meshesFG.resize(chunkCount, nullptr);
    }

    for (int chunk=0; chunk<chunkCount; ++chunk)
    {
        if (!meshesBG[chunk])
        {
            Geometry geo;
            Vec3 loc{0.f, 0.f, 0.f};

            Geometry::Vertex vert;
            Geometry::Triangle tri;

            vert.norm = Geometry::Vec3{0.f, 0.f, 1.f};

            for (int i=0; i<chunkSize && (chunk*chunkSize+i)<(width*height); ++i)
            {
                int x = (chunk*chunkSize+i)%width;
                int y = (chunk*chunkSize+i)/width;

                loc = Vec3{x*8.f, y*8.f,-5.f};

                vert.pos = loc;
                vert.tex = Geometry::Vec2{(Tile::fromBG().val%16)/16.f, (16-Tile::fromBG().val/16-1)/16.f};
                tri[0] = addOnce(geo.vertices, vert);

                vert.pos.y += 8.f;
                vert.tex = Geometry::Vec2{(Tile::fromBG().val%16)/16.f, (16-Tile::fromBG().val/16)/16.f};
                tri[1] = addOnce(geo.vertices, vert);

                vert.pos.x += 8.f;
                vert.tex = Geometry::Vec2{(Tile::fromBG().val%16+1)/16.f, (16-Tile::fromBG().val/16)/16.f};
                tri[2] = addOnce(geo.vertices, vert);

                geo.triangles.push_back(tri);

                vert.pos.y -= 8.f;
                vert.tex = Geometry::Vec2{(Tile::fromBG().val%16+1)/16.f, (16-Tile::fromBG().val/16-1)/16.f};
                tri[1] = addOnce(geo.vertices, vert);

                geo.triangles.push_back(tri);
            }

            meshesBG[chunk] = new Mesh(geo);
        }

        if (!meshesFG[chunk])
        {
            Geometry geo;
            Vec3 loc{0.f, 0.f, 0.f};

            Geometry::Vertex vert;
            Geometry::Triangle tri;

            vert.norm = Geometry::Vec3{0.f, 0.f, 1.f};

            for (int i=0; i<chunkSize && (chunk*chunkSize+i)<(width*height); ++i)
            {
                int x = (chunk*chunkSize+i)%width;
                int y = (chunk*chunkSize+i)/width;

                if (tileAt(x,y).val == 0) continue;

                loc = Vec3{x*8.f, y*8.f,-5.f};

                vert.pos = loc;
                vert.tex = Geometry::Vec2{(tileAt(x,y).val%16)/16.f, (16-tileAt(x,y).val/16-1)/16.f};
                tri[0] = addOnce(geo.vertices, vert);

                vert.pos.y += 8.f;
                vert.tex = Geometry::Vec2{(tileAt(x,y).val%16)/16.f, (16-tileAt(x,y).val/16)/16.f};
                tri[1] = addOnce(geo.vertices, vert);

                vert.pos.x += 8.f;
                vert.tex = Geometry::Vec2{(tileAt(x,y).val%16+1)/16.f, (16-tileAt(x,y).val/16)/16.f};
                tri[2] = addOnce(geo.vertices, vert);

                geo.triangles.push_back(tri);

                vert.pos.y -= 8.f;
                vert.tex = Geometry::Vec2{(tileAt(x,y).val%16+1)/16.f, (16-tileAt(x,y).val/16-1)/16.f};
                tri[1] = addOnce(geo.vertices, vert);

                geo.triangles.push_back(tri);
            }

            meshesFG[chunk] = new Mesh(geo);
        }
    }
}
