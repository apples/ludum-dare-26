#ifndef IMAGEBANK_H
#define IMAGEBANK_H

#include "inugami/texture.hpp"
#include "inugami/spritesheet.hpp"

#include <map>
#include <string>

class ImageBank
{
    ImageBank();
public:
    ~ImageBank();

    static ImageBank& getInstance();

    Inugami::Texture& getTexture(const std::string& name);
    Inugami::Spritesheet& getSheet(const std::string& name);

private:
    std::map<std::string,Inugami::Texture*> textures;
    std::map<std::string,Inugami::Spritesheet*> sheets;
};

#endif // IMAGEBANK_H
