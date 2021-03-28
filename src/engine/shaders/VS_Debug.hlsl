#define VERTEX_SHADER

#include "Common.hlsli"
#include "ConstantBuffers.hlsli"

struct VertexDebug {
    float3  pos     : POSITION;
    float4  color   : COLOR;
};

VSOutputDebug main(in VertexDebug IN) {
    VSOutputDebug OUT;

    OUT.pos = mul(float4(IN.pos, 1.0f), Matrices_ModelViewProj);
    OUT.color = IN.color;

    return OUT;
}
