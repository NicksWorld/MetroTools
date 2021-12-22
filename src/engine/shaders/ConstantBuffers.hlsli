cbuffer CBuffer_Matrices : register(b0) {
    row_major matrix Matrices_Model;
    row_major matrix Matrices_View;
    row_major matrix Matrices_Projection;
    row_major matrix Matrices_ModelView;
    row_major matrix Matrices_ModelViewProj;
    row_major matrix Matrices_NormalWS;
};

#if defined(VERTEX_SHADER)

cbuffer CBuffer_Skinned : register(b1) {
    float4           Skinned_VScale;
    row_major matrix Skinned_Bones[256];
};

cbuffer CBuffer_Terrain : register(b2) {
    float4          Terrain_Params0;    // x - hmap width - 1, y - hmap height - 1, z - num chunks X, w - num chunks Y
    float4          Terrain_Params1;    // xyz - terrain min, w - vacant
    float4          Terrain_Params2;    // xyz - terrain dim, w - vacant
};

#elif defined(PIXEL_SHADER)

cbuffer CBuffer_SurfParams : register(b1) {
    float4           Surf_Params0;      // x - alpha cut, w - render mode
};

#endif
