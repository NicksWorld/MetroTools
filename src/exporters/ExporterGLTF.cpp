#include "ExporterGLTF.h"
#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroLevel.h"
#include "metro/MetroContext.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_USE_CPP14
#define JSON_NOEXCEPTION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"


static MyArray<MetroVertex> MakeCommonVertices(const MetroModelGeomData& gd) {
    MyArray<MetroVertex> result;

    if (gd.mesh->vertexType == MetroVertexType::Skin) {
        const VertexSkinned* srcVerts = rcast<const VertexSkinned*>(gd.vertices);

        result.resize(gd.mesh->verticesCount);
        MetroVertex* dstVerts = result.data();

        for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
            *dstVerts = ConvertVertex(*srcVerts);
            dstVerts->pos *= gd.mesh->verticesScale;

            //#NOTE_SK: need to remap bones
            dstVerts->bones[0] = gd.mesh->bonesRemap[srcVerts->bones[0] / 3];
            dstVerts->bones[1] = gd.mesh->bonesRemap[srcVerts->bones[1] / 3];
            dstVerts->bones[2] = gd.mesh->bonesRemap[srcVerts->bones[2] / 3];
            dstVerts->bones[3] = gd.mesh->bonesRemap[srcVerts->bones[3] / 3];

            ++srcVerts;
            ++dstVerts;
        }
    } else if (gd.mesh->vertexType == MetroVertexType::Static) {
        const VertexStatic* srcVerts = rcast<const VertexStatic*>(gd.vertices);

        result.resize(gd.mesh->verticesCount);
        MetroVertex* dstVerts = result.data();

        for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
            *dstVerts = ConvertVertex(*srcVerts);
            ++srcVerts;
            ++dstVerts;
        }
    } else if (gd.mesh->vertexType == MetroVertexType::Soft) {
        const VertexSoft* srcVerts = rcast<const VertexSoft*>(gd.vertices);

        result.resize(gd.mesh->verticesCount);
        MetroVertex* dstVerts = result.data();

        for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
            *dstVerts = ConvertVertex(*srcVerts);
            ++srcVerts;
            ++dstVerts;
        }
    } else {
        assert(false && "Unknown vertex type!");
    }

    return result;
}


ExporterGLTF::ExporterGLTF()
    : mExcludeCollision(false)
    , mExportMesh(true)
    , mExportSkeleton(false)
    , mExportAnimation(false)
    , mExportMotionIdx(kInvalidValue) {
    mExporterName = "MetroEX";
}
ExporterGLTF::~ExporterGLTF() {
}

void ExporterGLTF::SetExporterName(const CharString& exporterName) {
    mExporterName = exporterName;
}

void ExporterGLTF::SetTexturesExtension(const CharString& ext) {
    mTexturesExtension = ext;
}

void ExporterGLTF::SetExcludeCollision(const bool b) {
    mExcludeCollision = b;
}

void ExporterGLTF::SetExportMesh(const bool b) {
    mExportMesh = b;
}

void ExporterGLTF::SetExportSkeleton(const bool b) {
    mExportSkeleton = b;
}

void ExporterGLTF::SetExportAnimation(const bool b) {
    mExportAnimation = b;
}

void ExporterGLTF::SetExportMotionIdx(const size_t idx) {
    mExportMotionIdx = idx;
}

bool ExporterGLTF::ExportModel(const MetroModelBase& model, const fs::path& filePath) {
    tinygltf::Model gltfModel;
    tinygltf::Scene gltfScene;

    mTexturesFolder = filePath.parent_path();
    CharString modelName = filePath.stem().string();

    // Create a simple material
    tinygltf::Material gltfMaterial;
    gltfMaterial.pbrMetallicRoughness.baseColorFactor = { 1.0f, 0.9f, 0.9f, 1.0f };
    gltfMaterial.doubleSided = false;
    gltfModel.materials.push_back(gltfMaterial);

    MyArray<MetroModelGeomData> gds;
    model.CollectGeomData(gds);

    size_t meshIdx = 0;
    for (auto& gd : gds) {
        const int gltfMeshIdx = scast<int>(meshIdx);

        MyArray<MetroVertex> vertices = MakeCommonVertices(gd);

        tinygltf::Buffer gltfIB;
        gltfIB.data.resize(scast<size_t>(gd.mesh->facesCount) * 3 * sizeof(uint16_t));
        memcpy(gltfIB.data.data(), gd.faces, gltfIB.data.size());

        tinygltf::Buffer gltfVB;
        gltfVB.data.resize(vertices.size() * sizeof(MetroVertex));
        memcpy(gltfVB.data.data(), vertices.data(), gltfVB.data.size());

        tinygltf::BufferView gltfIBView;
        gltfIBView.buffer = gltfMeshIdx * 2;
        gltfIBView.byteOffset = 0;
        gltfIBView.byteLength = gltfIB.data.size();
        gltfIBView.byteStride = 0;
        gltfIBView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

        tinygltf::BufferView gltfVBView;
        gltfVBView.buffer = gltfMeshIdx * 2 + 1;
        gltfVBView.byteOffset = 0;
        gltfVBView.byteLength = gltfVB.data.size();
        gltfVBView.byteStride = sizeof(MetroVertex);
        gltfVBView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

        tinygltf::Accessor gltfIBAccessor;
        gltfIBAccessor.bufferView = gltfMeshIdx * 2;
        gltfIBAccessor.byteOffset = 0;
        gltfIBAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
        gltfIBAccessor.type = TINYGLTF_TYPE_SCALAR;
        gltfIBAccessor.count = scast<size_t>(gd.mesh->facesCount) * 3;

        tinygltf::Accessor gltfVBAccessorPos;
        gltfVBAccessorPos.bufferView = gltfMeshIdx * 2 + 1;
        gltfVBAccessorPos.byteOffset = 0;
        gltfVBAccessorPos.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        gltfVBAccessorPos.type = TINYGLTF_TYPE_VEC3;
        gltfVBAccessorPos.count = vertices.size();

        tinygltf::Accessor gltfVBAccessorNormal;
        gltfVBAccessorNormal.bufferView = gltfMeshIdx * 2 + 1;
        gltfVBAccessorNormal.byteOffset = offsetof(MetroVertex, normal);
        gltfVBAccessorNormal.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        gltfVBAccessorNormal.type = TINYGLTF_TYPE_VEC3;
        gltfVBAccessorNormal.count = vertices.size();

        tinygltf::Accessor gltfVBAccessorUV;
        gltfVBAccessorUV.bufferView = gltfMeshIdx * 2 + 1;
        gltfVBAccessorUV.byteOffset = offsetof(MetroVertex, uv0);
        gltfVBAccessorUV.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        gltfVBAccessorUV.type = TINYGLTF_TYPE_VEC2;
        gltfVBAccessorUV.count = vertices.size();

        tinygltf::Primitive gltfPrimitive;
        gltfPrimitive.indices = gltfMeshIdx * 2;                        // The index of the accessor for the vertex indices
        gltfPrimitive.attributes["POSITION"] = gltfMeshIdx * 2 + 1;     // The index of the accessor for positions
        gltfPrimitive.attributes["NORMAL"] = gltfMeshIdx * 2 + 2;       // The index of the accessor for normals
        gltfPrimitive.attributes["TEXCOORD_0"] = gltfMeshIdx * 2 + 3;   // The index of the accessor for texcoords
        gltfPrimitive.material = 0;
        gltfPrimitive.mode = TINYGLTF_MODE_TRIANGLES;

        tinygltf::Mesh gltfMesh;
        gltfMesh.primitives.push_back(gltfPrimitive);
        gltfMesh.name = CharString("mesh_") + std::to_string(meshIdx++);

        tinygltf::Node gltfNode;
        gltfNode.mesh = gltfMeshIdx;
        gltfScene.nodes.push_back(gltfMeshIdx);

        gltfModel.meshes.push_back(gltfMesh);
        gltfModel.nodes.push_back(gltfNode);
        gltfModel.buffers.push_back(gltfIB);
        gltfModel.buffers.push_back(gltfVB);
        gltfModel.bufferViews.push_back(gltfIBView);
        gltfModel.bufferViews.push_back(gltfVBView);
        gltfModel.accessors.push_back(gltfIBAccessor);
        gltfModel.accessors.push_back(gltfVBAccessorPos);
        gltfModel.accessors.push_back(gltfVBAccessorNormal);
        gltfModel.accessors.push_back(gltfVBAccessorUV);
    }

    gltfModel.scenes.push_back(gltfScene);
    gltfModel.asset.version = "2.0";
    gltfModel.asset.generator = mExporterName;

    tinygltf::TinyGLTF gltf;
    return gltf.WriteGltfSceneToFile(&gltfModel, filePath.u8string(), false, true, true, false);
}

bool ExporterGLTF::ExportLevel(const MetroLevel& level, const fs::path& filePath) {
    return false;
}

const StringArray& ExporterGLTF::GetUsedTextures() const {
    return mUsedTextures;
}

const fs::path& ExporterGLTF::GetTexturesFolder() const {
    return mTexturesFolder;
}
