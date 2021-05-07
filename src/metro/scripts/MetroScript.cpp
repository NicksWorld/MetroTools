#include "MetroScript.h"
#include "metro/reflection/MetroReflection.h"
#include "metro/MetroConfigNames.h"

void MetroScriptGroup::Serialize(MetroReflectionStream& s) {
    assert(false);
}

void MetroScriptBlocks::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, version);
    METRO_SERIALIZE_MEMBER(s, block_count);
    if (block_count == ~0u) {
        METRO_SERIALIZE_NAMED_MEMBER(s, block_count, count);
    }
    blocks.resize(block_count);
    for (uint32_t i = 0; i != block_count; i++) {
        auto sect = s.OpenSection(kEmptyString);
        *sect >> blocks[i];
        s.CloseSection(sect);
    }
}

void MetroScript::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, groups);
    METRO_SERIALIZE_STRUCT_MEMBER(s, blocks);
    METRO_SERIALIZE_MEMBER(s, link_count);
    links.resize(link_count);
    for (uint32_t i = 0; i != link_count; i++) {
        char buf[10];
        sprintf(buf, "%lu", i);
        METRO_SERIALIZE_NAMED_MEMBER(s, links[i], buf);
    }
}

void MetroScriptRef::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, vs_name);
    METRO_SERIALIZE_MEMBER(s, vs_debug);
    METRO_SERIALIZE_MEMBER(s, vs_active);
    METRO_SERIALIZE_MEMBER(s, disable_qsave);
    METRO_SERIALIZE_MEMBER(s, save_on_nextlevel);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, vs_ref);
    bool ok = ConfigNamesDB::Get().AddName("content\\scripts\\" + vs_ref + ".bin");
    if (ok) {
        LogPrintF(LogLevel::Info, "VS ref: %s", vs_ref.c_str());
    }
    METRO_SERIALIZE_MEMBER(s, vs_ref_dyn_state_exist);
    assert(vs_ref_dyn_state_exist == false);
    if (!vs_ref.empty()) {
//        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, exposed_blocks);
    }
}
