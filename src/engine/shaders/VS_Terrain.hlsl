#define VERTEX_SHADER
#define VTX_TYPE_TERRAIN

#include "Common.hlsli"
#include "ConstantBuffers.hlsli"
#include "VertexCommon.hlsli"

struct VertexTerrain {
    float2  chunkXY : INSTDATA0;
};

Texture2D TexHMap   : register(t0);

static const uint kTerrainChunkWidth = 128;

VSOutputTerrain main(in VertexTerrain IN, in uint vertexID : SV_VertexID) {
    VSOutputTerrain OUT = (VSOutputTerrain)0;

    float innerX = float(vertexID % kTerrainChunkWidth);
    float innerY = float(vertexID / kTerrainChunkWidth);

    float vposRescale = float(kTerrainChunkWidth) / float(kTerrainChunkWidth - 1);

    float2 hmapXY = mad(float2(innerX, innerY), vposRescale, IN.chunkXY);
    float2 uv = hmapXY / Terrain_Params0.xy;

    float height = TexHMap.SampleLevel(SamplerPoint, uv, 0).x;

    float4 pos = float4(float3(uv.x, height, uv.y) * Terrain_Params2.xyz + Terrain_Params1.xyz, 1.0f);

    OUT.pos = mul(FromMetroV4(pos), Matrices_ModelViewProj);
    OUT.uv = uv;
    OUT.innerUV = float2(innerX, innerY) / float(kTerrainChunkWidth - 1);

    return OUT;
}
