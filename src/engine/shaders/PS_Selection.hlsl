#define PIXEL_SHADER
#include "Common.hlsli"
#include "ConstantBuffers.hlsli"

float4 main(VSOutput IN) : SV_Target0 {
    return Surf_Params0;
}
