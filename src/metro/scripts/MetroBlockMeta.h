#pragma once

#include "metro/MetroTypes.h"
#include <variant>

using U8Array = MyArray<uint8_t>;
using ParamValue = std::variant<uint16_t, bool, uint32_t, CharString, float, flags8, EntityLink, int, color4f, vec2, U8Array>;
enum ParamType {
    Type_u16, // "u16"
    Type_bool, // "bool"
    Type_bool8, // "bool8"
    Type_time, // "time" | "u32"
    Type_anim,
    Type_choose, // "choose" | "stringz"
    Type_color_u32, // "color, u32"
    Type_entity,
    Type_fp32, // "fp32"
    Type_sz, // "stringz"
    Type_part,
    Type_u8, // "u8"
    Type_u32, // "u32"
    Type_s32, // "s32"
    Type_color_vec4f,
    Type_vec2f,
    Type_u8_array,
};
struct MetaProp {
    ParamType   type;
    const char* name;
};
struct MetaInfo {
    MyArray<MetaProp>    props;
    MyArray<const char*> input;
    MyArray<const char*> output;
};
