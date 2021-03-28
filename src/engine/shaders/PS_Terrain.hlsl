#define PIXEL_SHADER
#include "Common.hlsli"
#include "ConstantBuffers.hlsli"

Texture2D TexDiffuse    : register(t0);
Texture2D TexNormal     : register(t1);
Texture2D TexLightmap   : register(t2);
Texture2D TexMask       : register(t3);
Texture2D TexDet0       : register(t4);
Texture2D TexDet1       : register(t5);
Texture2D TexDet2       : register(t6);
Texture2D TexDet3       : register(t7);

float2 FixUV(in float2 v) {
    return float2(v.x, -v.y);
}

PSOutput main(in VSOutputTerrain IN) {
    PSOutput OUT;

    float2 uv0 = IN.uv;
    float2 uv1 = IN.innerUV;

    float4 diffuse = TexDiffuse.Sample(SamplerAnisotropic, FixUV(uv0));
    float4 normal = TexNormal.Sample(SamplerTrilinear, FixUV(uv0));
    float lmap = TexLightmap.Sample(SamplerTrilinear, uv0).x;
    float4 mask = TexMask.Sample(SamplerTrilinear, FixUV(uv0));
    float4 det0 = TexDet0.Sample(SamplerAnisotropic, uv1 * 3);
    float4 det1 = TexDet1.Sample(SamplerAnisotropic, uv1 * 3);
    float4 det2 = TexDet2.Sample(SamplerAnisotropic, uv1 * 3);
    float4 det3 = TexDet3.Sample(SamplerAnisotropic, uv1 * 3);

    float4 detFinal = (det0 * mask.r + det1 * mask.g + det2 * mask.b + det3 * mask.a) / max(0.0001f, mask.r + mask.g + mask.b + mask.a);

    OUT.albedo = float4(diffuse.xyz * detFinal.xyz, lmap);
    OUT.normal = normal.xzy;

    return OUT;
}
