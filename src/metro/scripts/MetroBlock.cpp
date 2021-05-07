#include "MetroBlock.h"
#include "metro/reflection/MetroReflection.h"
#include "MetroBlockFactory.h"

Block::Block() {
}

Block::Block(uint32_t clsid, const char* name, const MetaInfo* meta)
    : clsid(clsid)
    , name(name)
    , meta(meta) {
}

#define METRO_SERIALIZE_MEMBER_V(s, type)                        \
    {                                                            \
        type v;                                                  \
        s.SerializeTypeInfo(el.name, MetroTypeGetAlias<type>()); \
        s >> v;                                                  \
        value = v;                                               \
    }

#define METRO_SERIALIZE_ARRAY_MEMBER_V(s, type)                                                               \
    {                                                                                                         \
        type v;                                                                                               \
        (s).SerializeTypeInfo(el.name, MetroTypeArrayGetAlias<ArrayElementTypeGetter<type>::elem_type>());    \
        (s).SerializeArray<ArrayElementTypeGetter<type>::elem_type, uint32_t>(v);                             \
        value = v;                                                                                            \
    }

void Block::Read(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, posx);
    METRO_SERIALIZE_MEMBER(s, posy);
    if (meta) {
        for (const auto& el : meta->props) {
            ParamValue value;
            switch (el.type) {
            case Type_u16:
                METRO_SERIALIZE_MEMBER_V(s, uint16_t)
                break;
            case Type_bool:
                METRO_SERIALIZE_MEMBER_V(s, bool)
                break;
            case Type_bool8:
                METRO_SERIALIZE_MEMBER_V(s, flags8)
                break;
            case Type_time:
                METRO_SERIALIZE_MEMBER_V(s, uint32_t)
                break;
            case Type_anim:
                METRO_SERIALIZE_MEMBER_V(s, CharString)
                break;
            case Type_choose:
                METRO_SERIALIZE_MEMBER_V(s, CharString)
                break;
            case Type_color_u32:
                METRO_SERIALIZE_MEMBER_V(s, uint32_t)
                break;
            case Type_entity:
                METRO_SERIALIZE_MEMBER_V(s, EntityLink)
                break;
            case Type_fp32:
                METRO_SERIALIZE_MEMBER_V(s, float)
                break;
            case Type_sz:
                METRO_SERIALIZE_MEMBER_V(s, CharString)
                break;
            case Type_part:
                METRO_SERIALIZE_MEMBER_V(s, CharString)
                break;
            case Type_u8:
                METRO_SERIALIZE_MEMBER_V(s, uint8_t)
                break;
            case Type_u32:
                METRO_SERIALIZE_MEMBER_V(s, uint32_t)
                break;
            case Type_s32:
                METRO_SERIALIZE_MEMBER_V(s, int32_t)
                break;
            case Type_color_vec4f:
                METRO_SERIALIZE_MEMBER_V(s, color4f)
                break;
            case Type_vec2f:
                METRO_SERIALIZE_MEMBER_V(s, vec2)
                break;
            case Type_u8_array:
                METRO_SERIALIZE_ARRAY_MEMBER_V(s, U8Array)
                break;
            default:
                assert(false);
            }
            params.emplace_back(el.name, value);
        }
    } else {
        unknown.resize(s.GetRemains());
        s.SerializeRawBytes(unknown.data(), unknown.size());
    }
}

#undef METRO_SERIALIZE_MEMBER_V

void Block::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, clsid);
    *this = BlockFactory::Create(clsid);
    Read(s);
}

void BlockRef::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, blkid);
    Block::Serialize(s);
}
