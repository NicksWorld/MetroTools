#define PIXEL_SHADER
#include "Common.hlsli"

float4 main(in VSOutputDebug IN) : SV_Target0 {
    return IN.color;
}
