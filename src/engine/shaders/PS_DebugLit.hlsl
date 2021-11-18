#define PIXEL_SHADER
#include "Common.hlsli"

float4 main(in VSOutputDebug2 IN) : SV_Target0 {
    const float3 lightDir = normalize(float3(0.35f, 0.75f, 0.35f));

    float3 vertexNormal = normalize(IN.normal.xyz);

    float diffuse = max(0.35f, dot(vertexNormal, lightDir));

    float4 color = float4(IN.color.xyz * diffuse, IN.color.a);

    return color;
}
