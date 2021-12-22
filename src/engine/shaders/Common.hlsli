// Vertex shader output
struct VSOutput {
    float4 pos          : SV_Position;
    float4 wpos         : TEXCOORD0;    // W is vacant
    float4 normal       : TEXCOORD1;    // vao in W
    float4 uv0uv1       : TEXCOORD2;
};

struct VSOutputSimple {
    float4 pos          : SV_Position;
    float2 uv           : TEXCOORD0;
};

struct VSOutputTerrain {
    float4 pos          : SV_Position;
    float2 uv           : TEXCOORD0;
    float2 innerUV      : TEXCOORD1;
};

struct VSOutputDebug {
    float4 pos          : SV_Position;
    float4 color        : TEXCOORD0;
};

struct VSOutputDebug2 {
    float4 pos          : SV_Position;
    float4 normal       : TEXCOORD0;
    float4 color        : TEXCOORD1;
};

struct PSOutput {
    float4 albedo       : SV_Target0;  // RGBA8_UNORM (RGB - albedo, A - interpolated vertex AO)
    float3 normal       : SV_Target1;  // R11G11B10 (world space)
    //float4 metalRough   : SV_Target2;  // RGBA8_UNORM (R - metalness, G - roughness)
};


// built-in samplers
SamplerState SamplerPoint       : register(s0);
SamplerState SamplerTrilinear   : register(s1);
SamplerState SamplerAnisotropic : register(s2);

#ifdef PIXEL_SHADER
struct GBuffer {
    float3  albedo;
    float3  normal;
    float   metalness;
    float   roughness;
    float   ao;
};

GBuffer UnpackGBuffer(Texture2D tex0, Texture2D tex1, Texture2D tex2, float2 uv) {
    float4 albedo_ao = tex0.Sample(SamplerPoint, uv);
    float3 normal = tex1.Sample(SamplerPoint, uv).xyz;

    GBuffer gbuffer;
    gbuffer.albedo = albedo_ao.xyz;
    gbuffer.normal = normalize(mad(normal, 2.0f, -1.0f));
    gbuffer.metalness = 0.0f;
    gbuffer.roughness = 0.0f;
    gbuffer.ao = albedo_ao.w;

    return gbuffer;
}
#endif // PIXEL_SHADER
