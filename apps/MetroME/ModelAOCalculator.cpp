#include "metro/MetroModel.h"
#include <embree3/rtcore.h>
#include <tbb/tbb.h>

#ifdef max
#undef max
#endif

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

constexpr float     kRayStartBias = 0.001f;
constexpr float     kAOIntensity  = 1.0f;


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

static void AverageAO(MetroModelGeomData& gd, const MyArray<float>& aos) {
    const MetroFace* faces = rcast<const MetroFace*>(gd.faces);
    for (size_t i = 0; i < gd.mesh->facesCount; ++i) {
        float a = aos[faces[i].a];
        float b = aos[faces[i].b];
        float c = aos[faces[i].c];

        const float average = (a + b + c) / 3.0f;
        a = a + (average - a) * 0.5f;
        b = b + (average - b) * 0.5f;
        c = c + (average - c) * 0.5f;

        if (gd.mesh->vertexType == MetroVertexType::Skin) {
            VertexSkinned* dstVerts = (VertexSkinned*)gd.vertices;
            dstVerts[faces[i].a].normal = EncodedNormalSetAO(dstVerts[faces[i].a].normal, a);
            dstVerts[faces[i].b].normal = EncodedNormalSetAO(dstVerts[faces[i].b].normal, b);
            dstVerts[faces[i].c].normal = EncodedNormalSetAO(dstVerts[faces[i].c].normal, c);
        } else {
            VertexStatic* dstVerts = (VertexStatic*)gd.vertices;
            dstVerts[faces[i].a].normal = EncodedNormalSetAO(dstVerts[faces[i].a].normal, a);
            dstVerts[faces[i].b].normal = EncodedNormalSetAO(dstVerts[faces[i].b].normal, b);
            dstVerts[faces[i].c].normal = EncodedNormalSetAO(dstVerts[faces[i].c].normal, c);
        }
    }
}

static float CalcAOForVertex(const vec4& pos, const vec3& normal, const MyArray<vec3>& samples, RTCScene embreeScene) {
    float occlusion = 0.0f;

    vec3 tangent, bitangent;
    OrthonormalBasis(normal, tangent, bitangent);
    mat3 tbn(tangent, bitangent, normal);

    for (const vec3& s : samples) {
        vec3 sample = tbn * s;

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
        if (ray.tfar <= ray.tnear) { // hit
            occlusion += 1.0f;  //#TODO_SK: implement distance weighting ??? something simple like clamp(1.0f - (hitDistance / maximumRange))
        }
    }

    //#TODO_SK: make intensity configurable
    return Clamp(1.0f - ((occlusion * kAOIntensity) / scast<float>(samples.size())), 0.0f, 1.0f);
}

bool CalculateModelAO(RefPtr<MetroModelBase>& model, const size_t quality) {
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

    size_t numSamples = 256;    // low
    if (quality == 1) {         // normal
        numSamples = 512;
    } else if (quality == 2) {  // high
        numSamples = 1024;
    } else if (quality > 0) {   // ultra
        numSamples = 2048;
    }

    MyArray<vec3> samples(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
        const float u = RandomFloat01();
        const float v = RandomFloat01();
        samples[i] = SampleCosineHemisphere(u, v);//SampleDirectionHemisphere(u, v);
    }

    for (size_t gdIdx = 0; gdIdx < gds.size(); ++gdIdx) {
        MetroModelGeomData& gd = gds[gdIdx];
        const MyArray<EmbreeVertex>& vertices = vertexBuffers[gdIdx];

        MyArray<float> aos(gd.mesh->verticesCount);

        if (gd.mesh->vertexType == MetroVertexType::Skin) {
            VertexSkinned* dstVerts = (VertexSkinned*)gd.vertices;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, gd.mesh->verticesCount),
                [&gd, dstVerts, &vertices, &aos, &samples, embreeScene](const tbb::blocked_range<size_t>& r) {
                    for (size_t i = r.begin(); i < r.end(); ++i) {
                        vec3 n = vec3(DecodeNormal(dstVerts[i].normal));
                        aos[i] = CalcAOForVertex(vertices[i].pos, n, samples, embreeScene);
                    }
                });
        } else {
            VertexStatic* dstVerts = (VertexStatic*)gd.vertices;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, gd.mesh->verticesCount),
                [&gd, dstVerts, &vertices, &aos, &samples, embreeScene](const tbb::blocked_range<size_t>& r) {
                    for (size_t i = r.begin(); i < r.end(); ++i) {
                        vec3 n = vec3(DecodeNormal(dstVerts[i].normal));
                        aos[i] = CalcAOForVertex(vertices[i].pos, n, samples, embreeScene);
                    }
                });
        }

        AverageAO(gd, aos);
    }

    rtcReleaseScene(embreeScene);
    rtcReleaseDevice(embreeDevice);

    return true;
}
