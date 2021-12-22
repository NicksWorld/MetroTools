#define PIXEL_SHADER
#include "Common.hlsli"
#include "ConstantBuffers.hlsli"

Texture2D GBufAlbedo        : register(t0);
Texture2D GBufNormal        : register(t1);
Texture2D GBufMetalRough    : register(t2);

float4 main(VSOutputSimple IN) : SV_Target0 {
    GBuffer gbuffer = UnpackGBuffer(GBufAlbedo, GBufNormal, GBufMetalRough, IN.uv);

    float4 outPixel = float4(0, 0, 0, 1);

    const int renderMode = (int)Surf_Params0.w;

    if (0 == renderMode) {          // Albedo
        outPixel.xyz = gbuffer.albedo;
    } else if (1 == renderMode) {   // Normal
        outPixel.xyz = mad(gbuffer.normal, 0.5f, 0.5f);
    } else if (2 == renderMode) {   // Gloss
        outPixel.xyz = gbuffer.metalness.xxx;
    } else if (3 == renderMode) {   // Roughness
        outPixel.xyz = gbuffer.roughness.xxx;
    } else if (4 == renderMode) {   // AO
        outPixel.xyz = gbuffer.ao.xxx;
    }

    return outPixel;
}
