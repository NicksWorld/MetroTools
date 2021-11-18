#define GEOMETRY_SHADER
#include "Common.hlsli"
#include "ConstantBuffers.hlsli"

[maxvertexcount(3)]
void main(triangle VSOutputDebug IN[3], inout TriangleStream<VSOutputDebug2> OUT) {
    VSOutputDebug2 vertex;

    float3 d0 = IN[1].pos.xyz - IN[0].pos.xyz;
    float3 d1 = IN[2].pos.xyz - IN[0].pos.xyz;
    float3 normalMS = normalize(cross(d0, d1));
    vertex.normal = float4(mul(normalMS, (float3x3)Matrices_NormalWS), 0.0f);

    [unroll]
    for (int i = 0; i < 3; ++i) {
        vertex.pos = mul(IN[i].pos, Matrices_ModelViewProj);
        vertex.color = IN[i].color;
        OUT.Append(vertex);
    }
    OUT.RestartStrip();
}
