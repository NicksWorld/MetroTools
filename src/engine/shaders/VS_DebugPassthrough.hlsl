#define VERTEX_SHADER
#include "Common.hlsli"

struct VertexDebug {
    float3  pos     : POSITION;
    float4  color   : COLOR;
};

VSOutputDebug main(in VertexDebug IN) {
    VSOutputDebug OUT;

    OUT.pos = float4(IN.pos, 1.0f);
    OUT.color = IN.color;

    return OUT;
}
