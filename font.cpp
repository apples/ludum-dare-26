#include "font.hpp"

#include "game.hpp"
#include "meta.hpp"

#include "inugami/texture.hpp"

using namespace Inugami;

Font::Font(const Texture& img, int tw, int th, float cx, float cy)
    : Spritesheet(img, tw, th, cx, cy)
    , tileW(tw/8)
    , tileH(th/8)
    , centerX(1.f-cx)
    , centerY(1.f-cy)
{}

Font::Font(const Font& in)
    : Spritesheet(in)
    , tileW(in.tileW)
    , tileH(in.tileH)
    , centerX(in.centerX)
    , centerY(in.centerY)
{}

Font::Font(Font&& in)
    : Spritesheet(in)
    , tileW(in.tileW)
    , tileH(in.tileH)
    , centerX(in.centerX)
    , centerY(in.centerY)
{}

Font::~Font()
{}

void Font::drawString(const std::string& in, Transform mat) const
{
    if (tilesX != 16 || tilesY != 16) throw GameError("Font: Invalid spritesheet.");

    Texture bgtex(Image(1,1,{{85,85,85,255}}));
    Mesh bgmesh(Geometry::fromRect(tileW,tileH,centerX,centerY));

    mat.push();
    for (char c : in)
    {
        if (c == '\n')
        {
            mat.pop();
            mat.translate(Vec3{0.f, -tileH, 0.f});
            mat.push();
            continue;
        }

        bgtex.bind(0);
        Game::getInstance().modelMatrix(mat);
        bgmesh.draw();

        if (c == ' ')
        {
            mat.translate(Vec3{tileW, 0.f, 0.f});
            continue;
        }

        if (c == '\t')
        {
            mat.translate(Vec3{tileW*4, 0.f, 0.f});
            continue;
        }

        mat.push();
        mat.scale(Vec3{1.0/8.0, 1.0/8.0, 1.0});
        Game::getInstance().modelMatrix(mat);
        draw(c/16, c%16);
        mat.pop();
        mat.translate(Vec3{tileW, 0.f, 0.f});
    }
}
