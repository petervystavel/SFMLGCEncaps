#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <GCFile.h>
#ifndef NOJSON
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;
#endif // NOJSON


class Metadata
{
public:

    struct Image
    {
        std::string filename;
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
        uint8_t rotated;
    };

    struct Texture
    {
        std::string textureName;
        uint16_t texturesize;
        uint32_t imageCount;
        std::vector<Image> images;
    };

    struct Header
    {
        uint32_t magic;
        uint32_t totalImageCount;
        uint32_t texturesCount;
    };

    struct Data
    {
        Header header{};
        std::vector<Texture> textures;
    };

#ifndef NOJSON
    static Data jsonToMetadataFile(json data);
#endif // NOJSON

    static bool MetadataFileToStruct(GCFile file, Metadata::Data& metadata);
};