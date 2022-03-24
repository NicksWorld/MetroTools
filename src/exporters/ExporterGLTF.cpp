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

using MyGLTFMaterialsDict = MyDict<HashString, int>;

void CreateGLTFMaterials(const MyArray<MetroModelGeomData>& gds,
                         MyGLTFMaterialsDict& materialsDict,
                         tinygltf::Model& gltfModel,
                         const CharString& textureExtension) {
    int materialIdx = 0;
    for (auto& gd : gds) {
        const CharString& gdTexture = gd.model->GetMaterialString(MetroModelBase::kMaterialStringTexture);
        HashString gdTextureHash(gdTexture);
        if (materialsDict.find(gdTextureHash) == materialsDict.end()) {
            MetroSurfaceDescription surfaceSet = MetroContext::Get().GetTexturesDB().GetSurfaceSetFromName(gdTexture, false);
            const CharString& albedoName = surfaceSet.albedo;
            const CharString& normalmapName = surfaceSet.normalmap;

            CharString textureName = fs::path(albedoName).filename().string();
            CharString textureFileName = textureName + textureExtension;

            tinygltf::Image gltfImage;
            gltfImage.uri = textureFileName;

            tinygltf::Texture gltfTexture;
            gltfTexture.name = textureName;
            gltfTexture.source = materialIdx;
            gltfTexture.sampler = 0;

            gltfModel.images.push_back(gltfImage);
            gltfModel.textures.push_back(gltfTexture);

            tinygltf::Material gltfMaterial;
            gltfMaterial.name = textureName;
            gltfMaterial.pbrMetallicRoughness.baseColorFactor = { 1.0, 1.0, 1.0, 1.0 };
            gltfMaterial.pbrMetallicRoughness.baseColorTexture.index = materialIdx;
            gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord = 0;
            gltfMaterial.pbrMetallicRoughness.metallicFactor = 0.0;
            gltfMaterial.pbrMetallicRoughness.roughnessFactor = 1.0;
            gltfMaterial.doubleSided = true;
            gltfModel.materials.push_back(gltfMaterial);

            materialsDict[gdTextureHash] = materialIdx;

            materialIdx++;
        }
    }
}

void CreateGLTFSkeleton(const RefPtr<MetroSkeleton>& skeleton, tinygltf::Scene& gltfScene, tinygltf::Model& gltfModel) {
    tinygltf::Skin       gltfSkin;
    tinygltf::Buffer     gltfInverseBindMatricesBuffer;
    tinygltf::BufferView gltfInverseBindMatricesBufferView;
    tinygltf::Accessor   gltfInverseBindMatricesAccessor;

    const size_t numBones = skeleton->GetNumBones();

    gltfInverseBindMatricesBuffer.data.resize(numBones * sizeof(mat4));
    mat4* dstMatricesPtr = rcast<mat4*>(gltfInverseBindMatricesBuffer.data.data());
    for (size_t i = 0; i < numBones; ++i) {
        dstMatricesPtr[i] = skeleton->GetBoneFullTransformInv(i);
    }

    gltfInverseBindMatricesBufferView.buffer = 0;
    gltfInverseBindMatricesBufferView.byteOffset = 0;
    gltfInverseBindMatricesBufferView.byteLength = gltfInverseBindMatricesBuffer.data.size();

    gltfInverseBindMatricesAccessor.bufferView = 0;
    gltfInverseBindMatricesAccessor.byteOffset = 0;
    gltfInverseBindMatricesAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    gltfInverseBindMatricesAccessor.type = TINYGLTF_TYPE_MAT4;
    gltfInverseBindMatricesAccessor.count = numBones;

    gltfModel.buffers.push_back(gltfInverseBindMatricesBuffer);
    gltfModel.bufferViews.push_back(gltfInverseBindMatricesBufferView);
    gltfModel.accessors.push_back(gltfInverseBindMatricesAccessor);

    gltfSkin.name = "skin_01";
    gltfSkin.inverseBindMatrices = 0;

    // create bones nodes
    for (size_t i = 0; i < numBones; ++i) {
        tinygltf::Node gltfNode;
        gltfNode.name = skeleton->GetBoneName(i);
        gltfNode.rotation.push_back(skeleton->GetBoneRotation(i).x);
        gltfNode.rotation.push_back(skeleton->GetBoneRotation(i).y);
        gltfNode.rotation.push_back(skeleton->GetBoneRotation(i).z);
        gltfNode.rotation.push_back(skeleton->GetBoneRotation(i).w);

        gltfNode.translation.push_back(skeleton->GetBonePosition(i).x);
        gltfNode.translation.push_back(skeleton->GetBonePosition(i).y);
        gltfNode.translation.push_back(skeleton->GetBonePosition(i).z);

        gltfModel.nodes.push_back(gltfNode);
        if (skeleton->GetBoneParentIdx(i) == kInvalidValue) { // only root bones
            gltfScene.nodes.push_back(scast<int>(i));
        }
        gltfSkin.joints.push_back(scast<int>(i));
    }

    // now assign children
    for (size_t i = 0; i < numBones; ++i) {
        const size_t parentIdx = skeleton->GetBoneParentIdx(i);
        if (parentIdx != kInvalidValue) {
            gltfModel.nodes[parentIdx].children.push_back(scast<int>(i));
        }
    }

    gltfModel.skins.push_back(gltfSkin);
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
    RefPtr<MetroSkeleton> skeleton = model.IsSkeleton() ? rcast<const MetroModelSkeleton&>(model).GetSkeleton() : nullptr;

    tinygltf::Model gltfModel;
    tinygltf::Scene gltfScene;

    // common samnpler
    tinygltf::Sampler gltfSampler;
    gltfSampler.name = "trilinear";
    gltfSampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
    gltfSampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
    gltfSampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
    gltfSampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
    gltfModel.samplers.push_back(gltfSampler);

    mTexturesFolder = filePath.parent_path();
    CharString modelName = filePath.stem().string();

    MyArray<MetroModelGeomData> gds;
    model.CollectGeomData(gds);

    MyGLTFMaterialsDict materialsDict;
    CreateGLTFMaterials(gds, materialsDict, gltfModel, mTexturesExtension);

    int bufViewAccessAddition = 0;
    if (skeleton) {
        CreateGLTFSkeleton(skeleton, gltfScene, gltfModel);
        // we've added buffer + view + accessor for inverseBindMatrices, so have to offset everything below
        bufViewAccessAddition = 1;
    }

    int gltfMeshIdx = 0;
    for (auto& gd : gds) {
        MyArray<MetroVertex> vertices = MakeCommonVertices(gd);

        auto materialIt = materialsDict.find(HashString(gd.model->GetMaterialString(MetroModelBase::kMaterialStringTexture)));
        const int materialIdx = (materialIt == materialsDict.end()) ? 0 : materialIt->second;

        tinygltf::Buffer gltfIB;
        gltfIB.data.resize(scast<size_t>(gd.mesh->facesCount) * 3 * sizeof(uint16_t));
        memcpy(gltfIB.data.data(), gd.faces, gltfIB.data.size());

        tinygltf::Buffer gltfVB;
        gltfVB.data.resize(vertices.size() * sizeof(MetroVertex));
        memcpy(gltfVB.data.data(), vertices.data(), gltfVB.data.size());

        tinygltf::BufferView gltfIBView;
        gltfIBView.buffer = gltfMeshIdx * 2 + bufViewAccessAddition;
        gltfIBView.byteOffset = 0;
        gltfIBView.byteLength = gltfIB.data.size();
        gltfIBView.byteStride = 0;
        gltfIBView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

        tinygltf::BufferView gltfVBView;
        gltfVBView.buffer = gltfMeshIdx * 2 + 1 + bufViewAccessAddition;
        gltfVBView.byteOffset = 0;
        gltfVBView.byteLength = gltfVB.data.size();
        gltfVBView.byteStride = sizeof(MetroVertex);
        gltfVBView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

        tinygltf::Accessor gltfIBAccessor;
        gltfIBAccessor.bufferView = gltfMeshIdx * 2 + bufViewAccessAddition;
        gltfIBAccessor.byteOffset = 0;
        gltfIBAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
        gltfIBAccessor.type = TINYGLTF_TYPE_SCALAR;
        gltfIBAccessor.count = scast<size_t>(gd.mesh->facesCount) * 3;

        tinygltf::Accessor gltfVBAccessorPos;
        gltfVBAccessorPos.bufferView = gltfMeshIdx * 2 + 1 + bufViewAccessAddition;
        gltfVBAccessorPos.byteOffset = 0;
        gltfVBAccessorPos.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        gltfVBAccessorPos.type = TINYGLTF_TYPE_VEC3;
        gltfVBAccessorPos.count = vertices.size();

        tinygltf::Accessor gltfVBAccessorNormal;
        gltfVBAccessorNormal.bufferView = gltfMeshIdx * 2 + 1 + bufViewAccessAddition;
        gltfVBAccessorNormal.byteOffset = offsetof(MetroVertex, normal);
        gltfVBAccessorNormal.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        gltfVBAccessorNormal.type = TINYGLTF_TYPE_VEC3;
        gltfVBAccessorNormal.count = vertices.size();

        tinygltf::Accessor gltfVBAccessorUV;
        gltfVBAccessorUV.bufferView = gltfMeshIdx * 2 + 1 + bufViewAccessAddition;
        gltfVBAccessorUV.byteOffset = offsetof(MetroVertex, uv0);
        gltfVBAccessorUV.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        gltfVBAccessorUV.type = TINYGLTF_TYPE_VEC2;
        gltfVBAccessorUV.count = vertices.size();

        tinygltf::Accessor gltfVBAccessorBones;
        tinygltf::Accessor gltfVBAccessorWeights;
        if (skeleton) {
            gltfVBAccessorBones.bufferView = gltfMeshIdx * 2 + 1 + bufViewAccessAddition;
            gltfVBAccessorBones.byteOffset = offsetof(MetroVertex, bones);
            gltfVBAccessorBones.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            gltfVBAccessorBones.type = TINYGLTF_TYPE_VEC4;
            gltfVBAccessorBones.count = vertices.size();

            gltfVBAccessorWeights.bufferView = gltfMeshIdx * 2 + 1 + bufViewAccessAddition;
            gltfVBAccessorWeights.byteOffset = offsetof(MetroVertex, weights);
            gltfVBAccessorWeights.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            gltfVBAccessorWeights.normalized = true;
            gltfVBAccessorWeights.type = TINYGLTF_TYPE_VEC4;
            gltfVBAccessorWeights.count = vertices.size();
        }

        const int numAccessors = (skeleton != nullptr) ? 6 : 4;

        tinygltf::Primitive gltfPrimitive;
        gltfPrimitive.indices = gltfMeshIdx * numAccessors + bufViewAccessAddition;                         // The index of the accessor for the vertex indices
        gltfPrimitive.attributes["POSITION"] = gltfMeshIdx * numAccessors + 1 + bufViewAccessAddition;      // The index of the accessor for positions
        gltfPrimitive.attributes["NORMAL"] = gltfMeshIdx * numAccessors + 2 + bufViewAccessAddition;        // The index of the accessor for normals
        gltfPrimitive.attributes["TEXCOORD_0"] = gltfMeshIdx * numAccessors + 3 + bufViewAccessAddition;    // The index of the accessor for texcoords
        if (skeleton) {
            gltfPrimitive.attributes["JOINTS_0"] = gltfMeshIdx * numAccessors + 4 + bufViewAccessAddition;  // The index of the accessor for bones ids
            gltfPrimitive.attributes["WEIGHTS_0"] = gltfMeshIdx * numAccessors + 5 + bufViewAccessAddition; // The index of the accessor for bones weights
        }
        gltfPrimitive.material = materialIdx;
        gltfPrimitive.mode = TINYGLTF_MODE_TRIANGLES;

        tinygltf::Mesh gltfMesh;
        gltfMesh.primitives.push_back(gltfPrimitive);
        gltfMesh.name = CharString("mesh_") + std::to_string(gltfMeshIdx);

        tinygltf::Node gltfNode;
        gltfNode.mesh = gltfMeshIdx;
        if (skeleton) {
            gltfNode.skin = 0;
        }
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
        if (skeleton) {
            gltfModel.accessors.push_back(gltfVBAccessorBones);
            gltfModel.accessors.push_back(gltfVBAccessorWeights);
        }

        gltfMeshIdx++;
    }

    gltfModel.scenes.push_back(gltfScene);
    gltfModel.asset.version = "2.0";
    gltfModel.asset.generator = mExporterName;

    tinygltf::TinyGLTF gltf;
    const bool result = gltf.WriteGltfSceneToFile(&gltfModel, filePath.u8string(), false, true, true, false);

    if (result) {
        mUsedTextures.clear();
        for (auto& it : materialsDict) {
            mUsedTextures.push_back(it.first.str);
        }
    }

    return result;
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
