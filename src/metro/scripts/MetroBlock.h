#pragma once

#include "metro/MetroTypes.h"
#include "MetroBlockMeta.h"

class MetroReflectionStream;

struct Block {
    using Params = MyArray<std::pair<const char*, ParamValue>>;

    Block();
    Block(uint32_t clsid, const char* name, const MetaInfo* meta);

    uint32_t   clsid = 0;
    CharString name;
    int16_t   posx = 0;
    int16_t   posy = 0;
    Params     params;

    virtual void Read(MetroReflectionStream& cfg);
    void         Serialize(MetroReflectionStream& s);

    const MetaInfo* meta = nullptr;
    BytesArray      unknown;
};

struct BlockRef : public Block {
    uint16_t blkid = 0;

    void Serialize(MetroReflectionStream& s);
};
