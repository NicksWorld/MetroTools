#pragma once

#include "metro/MetroTypes.h"
#include "MetroBlock.h"
#include <array>

class MetroReflectionStream;

struct MetroScriptGroup {
    void Serialize(MetroReflectionStream& s);
};

struct MetroScriptBlocks {
    uint16_t       version;
    uint32_t       block_count;
    MyArray<Block> blocks;

    void Serialize(MetroReflectionStream& s);
};

struct MetroScript {
    MyArray<MetroScriptGroup> groups;
    MetroScriptBlocks         blocks;
    uint32_t             link_count;
    MyArray<vec4s16>     links;

    void Serialize(MetroReflectionStream& s);
};

struct MetroScriptRef {
    CharString     vs_name;
    bool           vs_debug;
    bool           vs_active;
    bool           disable_qsave;
    bool           save_on_nextlevel;
    CharString     vs_ref;
    bool           vs_ref_dyn_state_exist;
 //   MyArray<BlockRef> exposed_blocks;

    void Serialize(MetroReflectionStream& s);
};
