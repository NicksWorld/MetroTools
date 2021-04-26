#define VERTEX_SHADER
#define VTX_TYPE_SOFT

#include "Common.hlsli"
#include "ConstantBuffers.hlsli"
#include "VertexCommon.hlsli"

VSOutput main(in InputVertex IN) {
    VSOutput OUT = TransformVertex(IN);
    return OUT;
}