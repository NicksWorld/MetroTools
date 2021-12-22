#include "metro/MetroModel.h"
#include <embree3/rtcore.h>

//#NOTE_SK: Embree requires shared vertices to be padded to 16 bytes
//          so it's easier to just use vec4 aligned (for berret SIMD perf)
__declspec(align(16))
struct EmbreeVertex {
    vec4 pos;
};

// The index buffer must contain an array of three 32-bit indices per triangle (RTC_FORMAT_UINT3 format)
PACKED_STRUCT_BEGIN
struct EmbreeFace {
    uint32_t a, b, c;
} PACKED_STRUCT_END;

constexpr size_t    kNumSamples = 256;
constexpr float     kSampleWeight = 1.0f / scast<float>(kNumSamples);
constexpr float     kRayStartBias = 0.001f;


// Maps a value inside the square [0,1]x[0,1] to a value in a disk of radius 1 using concentric squares.
// This mapping preserves area, bi continuity, and minimizes deformation.
// Based off the algorithm "A Low Distortion Map Between Disk and Square" by Peter Shirley and
// Kenneth Chiu.
static vec2 SquareToConcentricDiskMapping(float x, float y) {
    float phi = 0.0f;
    float r = 0.0f;

    // -- (a,b) is now on [-1,1]
    float a = 2.0f * x - 1.0f;
    float b = 2.0f * y - 1.0f;

    if (a > -b) {                    // region 1 or 2
        if (a > b) {                 // region 1, also |a| > |b|
            r = a;
            phi = (MM_Pi / 4.0f) * (b / a);
        } else {                      // region 2, also |b| > |a|
            r = b;
            phi = (MM_Pi / 4.0f) * (2.0f - (a / b));
        }
    } else {                          // region 3 or 4
        if (a < b) {                 // region 3, also |a| >= |b|, a != 0
            r = -a;
            phi = (MM_Pi / 4.0f) * (4.0f + (b / a));
        } else {                      // region 4, |b| >= |a|, but a==0 and b==0 could occur.
            r = -b;
            if (b != 0) {
                phi = (MM_Pi / 4.0f) * (6.0f - (a / b));
            } else {
                phi = 0;
            }
        }
    }

    return vec2(r * Cos(phi), r * Sin(phi));
}

// Returns a random cosine-weighted direction on the hemisphere around z = 1
static vec3 SampleCosineHemisphere(float u1, float u2) {
    vec2 uv = SquareToConcentricDiskMapping(u1, u2);
    const float u = uv.x;
    const float v = uv.y;

    // Project samples on the disk to the hemisphere to get a
    // cosine weighted distribution
    const float r = u * u + v * v;
    return vec3(u, v, Sqrt(std::max(0.0f, 1.0f - r)));
}

static vec3 SampleDirectionHemisphere(float u1, float u2) {
    float z = u1;
    float r = Sqrt(std::max(0.0f, 1.0f - z * z));
    float phi = MM_TwoPi * u2;
    float x = r * Cos(phi);
    float y = r * Sin(phi);

    return vec3(x, y, z);
}

static float CalcAOForVertex(const vec4& pos, const vec3& normal, const MyArray<vec3>& samples, RTCScene embreeScene) {
    float result = 0.0f;

    vec3 tangent, bitangent;
    OrthonormalBasis(normal, tangent, bitangent);
    mat3 tbn(tangent, bitangent, normal);

    for (size_t i = 0; i < kNumSamples; ++i) {
        vec3 sample = tbn * samples[i];

        RTCIntersectContext intersectCtx;
        rtcInitIntersectContext(&intersectCtx);
        RTCRay ray;
        ray.mask = kInvalidValue32;
        ray.flags = 0;
        ray.time = 0.0f;
        ray.org_x = pos.x;
        ray.org_y = pos.y;
        ray.org_z = pos.z;
        ray.dir_x = sample.x;
        ray.dir_y = sample.y;
        ray.dir_z = sample.z;
        ray.tnear = kRayStartBias;
        ray.tfar = std::numeric_limits<float>::max();

        rtcOccluded1(embreeScene, &intersectCtx, &ray);
        if (ray.tfar > ray.tnear) { // no occlusion
            result += kSampleWeight;
        }
    }

    return Clamp(result, 0.0f, 1.0f);
}

bool CalculateModelAO(RefPtr<MetroModelBase>& model) {
    RTCDevice embreeDevice = rtcNewDevice(nullptr);
    RTCScene embreeScene = rtcNewScene(embreeDevice);

    MyArray<MetroModelGeomData> gds;
    model->CollectGeomData(gds);

    MyArray<MyArray<EmbreeVertex>> vertexBuffers(gds.size());
    MyArray<MyArray<EmbreeFace>> indexBuffers(gds.size());

    for (size_t gdIdx = 0; gdIdx < gds.size(); ++gdIdx) {
        const MetroModelGeomData& gd = gds[gdIdx];
        MyArray<EmbreeVertex>& vertices = vertexBuffers[gdIdx];
        MyArray<EmbreeFace>& faces = indexBuffers[gdIdx];
        vertices.resize(gd.mesh->verticesCount);
        faces.resize(gd.mesh->facesCount);

        if (gd.mesh->vertexType == MetroVertexType::Skin) {
            const VertexSkinned* srcVerts = rcast<const VertexSkinned*>(gd.vertices);
            for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
                vertices[i].pos = vec4(DecodeSkinnedPosition(srcVerts[i].pos) * gd.mesh->verticesScale, 0.0);
            }
        } else {
            const VertexStatic* srcVerts = rcast<const VertexStatic*>(gd.vertices);
            for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
                vertices[i].pos = vec4(srcVerts[i].pos, 0.0f);
            }
        }

        const MetroFace* srcFaces = rcast<const MetroFace*>(gd.faces);
        for (size_t i = 0; i < gd.mesh->facesCount; ++i) {
            faces[i].a = scast<uint32_t>(srcFaces[i].a);
            faces[i].b = scast<uint32_t>(srcFaces[i].b);
            faces[i].c = scast<uint32_t>(srcFaces[i].c);
        }

        RTCGeometry embreeGeom = rtcNewGeometry(embreeDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
        rtcSetGeometryBuildQuality(embreeGeom, RTC_BUILD_QUALITY_HIGH);
        rtcSetGeometryVertexAttributeCount(embreeGeom, 1);
        rtcSetSharedGeometryBuffer(embreeGeom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vertices.data(), 0, sizeof(EmbreeVertex), vertices.size());
        rtcSetSharedGeometryBuffer(embreeGeom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, faces.data(), 0, sizeof(EmbreeFace), faces.size());
        rtcCommitGeometry(embreeGeom);
        rtcAttachGeometry(embreeScene, embreeGeom);
        rtcReleaseGeometry(embreeGeom);
    }

    rtcCommitScene(embreeScene);

    MyArray<vec3> samples(kNumSamples);
    for (size_t i = 0; i < kNumSamples; ++i) {
        const float u = RandomFloat01();
        const float v = RandomFloat01();
        samples[i] = SampleCosineHemisphere(u, v);//SampleDirectionHemisphere(u, v);
    }

    for (size_t gdIdx = 0; gdIdx < gds.size(); ++gdIdx) {
        MetroModelGeomData& gd = gds[gdIdx];
        const MyArray<EmbreeVertex>& vertices = vertexBuffers[gdIdx];

        if (gd.mesh->vertexType == MetroVertexType::Skin) {
            VertexSkinned* dstVerts = (VertexSkinned*)gd.vertices;
            for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
                vec3 n = vec3(DecodeNormal(dstVerts[i].normal));
                const float ao = CalcAOForVertex(vertices[i].pos, n, samples, embreeScene);
                dstVerts[i].normal = EncodeNormal(n, ao);
            }
        } else {
            VertexStatic* dstVerts = (VertexStatic*)gd.vertices;
            for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
                vec3 n = vec3(DecodeNormal(dstVerts[i].normal));
                const float ao = CalcAOForVertex(vertices[i].pos, n, samples, embreeScene);
                dstVerts[i].normal = EncodeNormal(n, ao);
            }
        }
    }

    rtcReleaseScene(embreeScene);
    rtcReleaseDevice(embreeDevice);

    return true;
}
