#pragma once
#include "mycommon.h"
#include "mymath.h"

namespace u4a {

PACKED_STRUCT_BEGIN
struct VertexDebug {
    vec3        pos;
    color32u    color;
} PACKED_STRUCT_END;

} // namespace u4a
