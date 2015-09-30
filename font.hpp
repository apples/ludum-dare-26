#ifndef FONT_H
#define FONT_H

#include "inugami/spritesheet.hpp"
#include "inugami/transform.hpp"

class Font : public Inugami::Spritesheet
{
public:
    Font() = delete;
    Font(const Inugami::Texture& img, int tw, int th, float cx=0.f, float cy=0.f);
    Font(const Font& in);
    Font(Font&& in);
    virtual ~Font();

    void drawString(const std::string& in, Inugami::Transform mat) const;

private:
    int tileW, tileH;
    float centerX, centerY;
};

#endif // FONT_H
