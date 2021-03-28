// Input vertices (Metro)
struct VertexStatic {
    float3  pos     : POSITION;
    float4  normal  : NORMAL;
    float4  tangent : TANGENT;      // aux0
    float4  aux1    : COLOR;
    float2  uv      : TEXCOORD0;
};

struct VertexSkinned {
    uint4   pos     : POSITION;
    float4  normal  : NORMAL;
    float4  tangent : TANGENT;      // aux0
    float4  aux1    : COLOR;
    uint4   bones   : BONES;
    float4  weights : WEIGHTS;
    int2    uv      : TEXCOORD0;
};

struct VertexLevel {
    float3  pos     : POSITION;
    float4  normal  : NORMAL;
    float4  tangent : TANGENT;
    float4  aux1    : COLOR;
    int4    uv0uv1  : TEXCOORD0;
};



// Intermediate vertex representation
struct VertexCommon {
    float3  pos;
    float3  normal;
    float3  tangent;
    float3  bitangent;
    float2  uv0;
    float2  uv1;
    float   vao;
};


#define FromMetroV3(v)  ((v).zyx)
#define FromMetroV4(v)  ((v).zyxw)

#if defined(VTX_TYPE_STATIC)

#define InputVertex VertexStatic

VertexCommon ProcessInputVertex(in InputVertex src) {
    VertexCommon dst = (VertexCommon)0;

    float4 t = src.tangent * 2.0f - 1.0f;

    dst.pos = FromMetroV3(src.pos);
    dst.normal = FromMetroV3(src.normal) * 2.0f - 1.0f;
    dst.tangent = FromMetroV3(t);
    dst.bitangent = cross(dst.normal, dst.tangent) * t.w;
    dst.uv0 = src.uv;
    dst.vao = src.normal.w;

    return dst;
}

#endif // if defined(VTX_TYPE_STATIC)

#if defined(VTX_TYPE_SKINNED)

#define InputVertex VertexSkinned

VertexCommon ProcessInputVertex(in InputVertex src) {
    VertexCommon dst = (VertexCommon)0;

    const float kPositionDequant = 1.0f / 32767.0f;
    const float kUVDequant = 1.0f / 2048.0f;

    float4 t = src.tangent * 2.0f - 1.0f;

    // same logic as in Metro shaders:
    //   mul r1.yzw, r0.xxyz, l(0.000000, 0.000031, 0.000031, 0.000031)
    //   ge r8.xyz, l(32767.000000, 32767.000000, 32767.000000, 0.000000), r0.xyzx
    //   mad r0.xyz, r0.xyzx, l(0.000031, 0.000031, 0.000031, 0.000000), l(-2.000000, -2.000000, -2.000000, 0.000000)
    //   movc r0.xyz, r8.xyzx, r1.yzwy, r0.xyzx
    //   mul r0.xyz, r0.xyzx, v10.wwww
    dst.pos = float3(FromMetroV3(src.pos));
    dst.pos.x = (dst.pos.x > 32767.0f) ? (dst.pos.x * kPositionDequant - 2.0f) : (dst.pos.x * kPositionDequant);
    dst.pos.y = (dst.pos.y > 32767.0f) ? (dst.pos.y * kPositionDequant - 2.0f) : (dst.pos.y * kPositionDequant);
    dst.pos.z = (dst.pos.z > 32767.0f) ? (dst.pos.z * kPositionDequant - 2.0f) : (dst.pos.z * kPositionDequant);
    dst.pos *= Skinned_VScale.x;

    //#NOTE_SK: maybe better  "dst.pos = (dst.pos / 65535.0f) * 2.0f - 1.0f" ??

    dst.normal = FromMetroV3(src.normal) * 2.0f - 1.0f;
    dst.tangent = FromMetroV3(t);

    uint4 indices = FromMetroV4(src.bones);
    float4 weights = FromMetroV4(src.weights);
    matrix skinMat = Skinned_Bones[indices.x] * weights.x +
                     Skinned_Bones[indices.y] * weights.y +
                     Skinned_Bones[indices.z] * weights.z +
                     Skinned_Bones[indices.w] * weights.w;
    dst.pos = mul(float4(dst.pos, 1.0f), skinMat).xyz;
    dst.normal = mul(dst.normal, (float3x3)skinMat);
    dst.tangent = mul(dst.tangent, (float3x3)skinMat);

    dst.bitangent = cross(dst.normal, dst.tangent) * t.w;
    dst.uv0 = float2(src.uv) * kUVDequant;
    dst.vao = src.normal.w;

    return dst;
}

#endif // if defined(VTX_TYPE_SKINNED)

#if defined(VTX_TYPE_LEVEL)

#define InputVertex VertexLevel

VertexCommon ProcessInputVertex(in InputVertex src) {
    VertexCommon dst = (VertexCommon)0;

    const float kUV0Dequant = 1.0f / 1024.0f;
    const float kUV1Dequant = 1.0f / 32767.0f;

    float4 t = src.tangent * 2.0f - 1.0f;

    dst.pos = FromMetroV3(src.pos);
    dst.normal = FromMetroV3(src.normal) * 2.0f - 1.0f;
    dst.tangent = FromMetroV3(t);
    dst.bitangent = cross(dst.normal, dst.tangent) * t.w;

    dst.uv0 = src.uv0uv1.xy * kUV0Dequant;
    dst.uv1 = src.uv0uv1.zw * kUV1Dequant;

    dst.vao = 1.0f;

    return dst;
}

#endif // if defined(VTX_TYPE_LEVEL)



#if !defined(VTX_TYPE_TERRAIN)

VSOutput TransformVertex(in InputVertex src) {
    VSOutput dst = (VSOutput)0;

    VertexCommon vertex = ProcessInputVertex(src);

    dst.pos = mul(float4(vertex.pos, 1.0f), Matrices_ModelViewProj);
    dst.normal.xyz = normalize(mul(vertex.normal, (float3x3)Matrices_NormalWS));
    dst.tangent.xyz = normalize(mul(vertex.tangent, (float3x3)Matrices_NormalWS));
    dst.bitangent.xyz = normalize(mul(vertex.bitangent, (float3x3)Matrices_NormalWS));

    dst.normal.w = vertex.vao;

    dst.uv0uv1.xy = vertex.uv0;
    dst.uv0uv1.zw = vertex.uv1;

    return dst;
}

#endif // !defined(VTX_TYPE_TERRAIN)
