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

struct PSOutput {
    float4 albedo       : SV_Target0;  // RGBA8_UNORM (RGB - albedo, A - interpolated vertex AO)
    float3 normal       : SV_Target1;  // R11G11B10 (world space)
    //float4 metalRough   : SV_Target2;  // RGBA8_UNORM (R - metalness, G - roughness)
};


// built-in samplers
SamplerState SamplerPoint       : register(s0);
SamplerState SamplerTrilinear   : register(s1);
SamplerState SamplerAnisotropic : register(s2);
