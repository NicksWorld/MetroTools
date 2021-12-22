#define PIXEL_SHADER
#include "Common.hlsli"

Texture2D GBufAlbedo        : register(t0);
Texture2D GBufNormal        : register(t1);
Texture2D GBufMetalRough    : register(t2);

float4 main(VSOutputSimple IN) : SV_Target0 {
    const float3 lightDir = normalize(float3(0.35f, 0.75f, 0.35f));

    GBuffer gbuffer = UnpackGBuffer(GBufAlbedo, GBufNormal, GBufMetalRough, IN.uv);

    float diffuse = max(0.35f, dot(gbuffer.normal, lightDir));

    float4 color = float4(gbuffer.albedo * diffuse * gbuffer.ao, 1.0f);

    return color;
}
