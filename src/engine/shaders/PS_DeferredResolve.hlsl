#define PIXEL_SHADER
#include "Common.hlsli"

Texture2D GBufAlbedo        : register(t0);
Texture2D GBufNormal        : register(t1);
Texture2D GBufMetalRough    : register(t2);

float4 main(VSOutputSimple IN) : SV_Target0 {
    const float3 lightDir = normalize(float3(0.35f, 0.75f, 0.35f));

    float4 albedo_ao = GBufAlbedo.Sample(SamplerPoint, IN.uv);
    float3 normal = normalize(GBufNormal.Sample(SamplerPoint, IN.uv).xyz * 2.0f - 1.0f);

    float diffuse = max(0.35f, dot(normal, lightDir));

    float4 color = float4(albedo_ao.xyz * diffuse * albedo_ao.w, 1.0f);

    return color;
}
