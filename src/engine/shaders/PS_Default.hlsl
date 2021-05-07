#define PIXEL_SHADER
#include "Common.hlsli"
#include "ConstantBuffers.hlsli"

Texture2D TexAlbedo     : register(t0);
Texture2D TexNormalmap  : register(t1);
Texture2D TexBump       : register(t2);

PSOutput main(in VSOutput IN) {
    PSOutput OUT;

    float2 uv0 = float2(IN.uv0uv1.xy);

    float4 albedo = TexAlbedo.Sample(SamplerAnisotropic, uv0);
    clip(albedo.w - Surf_Params0.x);

    //float4 bump = TexBump.Sample(SamplerTrilinear, uv0);

    float3 vertexNormal = normalize(IN.normal.xyz);
    float3 worldPos = IN.wpos.xyz;
    float2 uvDdx = ddx_fine(uv0);
    float2 uvDdy = ddy_fine(uv0);
    float3 wpDdyN = cross(ddy_fine(worldPos), vertexNormal);
    float3 wpDdxN = cross(vertexNormal, ddx_fine(worldPos));
    float3 tangent = (wpDdyN * uvDdx.x) + (wpDdxN * uvDdy.x);
    float3 bitangent = (wpDdyN * uvDdx.y) + (wpDdxN * uvDdy.y);
    float il = rsqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
    tangent *= il;
    bitangent *= il;

    float3x3 TBN = float3x3(tangent, bitangent, vertexNormal);

    float3 normal = TexNormalmap.Sample(SamplerTrilinear, uv0).xyz * 2.0f - 1.0f;
    normal = normalize(mul(normal, TBN));

    OUT.albedo = float4(albedo.xyz, IN.normal.w);
    OUT.normal = normal * 0.5f + 0.5f;
    //OUT.metalRough = bump;

    return OUT;
}
