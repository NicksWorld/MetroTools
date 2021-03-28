struct ConstantBufferMatrices {
    mat4 Model;
    mat4 View;
    mat4 Projection;
    mat4 ModelView;
    mat4 ModelViewProj;
    mat4 NormalWS;
};

struct ConstantBufferSkinned {
    vec4 VScale;
    mat4 Bones[256];
};

struct ConstantBufferTerrain {
    vec4 Terrain_Params0;   // x - hmap width - 1, y - hmap height - 1, z - num chunks X, w - num chunks Y
    vec4 Terrain_Params1;   // xyz - terrain min, w - vacant
    vec4 Terrain_Params2;   // xyz - terrain dim, w - vacant
};

struct ConstantBufferSurfParams {
    vec4 param0;    // x - alpha cut
};
