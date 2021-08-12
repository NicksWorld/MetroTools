#include "ExporterFBX.h"
#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroLevel.h"
#include "metro/MetroContext.h"


#define FBXSDK_NEW_API
#define FBXSDK_SHARED
#include "fbxsdk.h"

#pragma comment(lib, "libfbxsdk.lib")


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

    return std::move(result);
}



using MyFbxMaterialsDict = MyDict<HashString, FbxSurfacePhong*>;

struct ClusterInfo {
    MyArray<int>      vertexIdxs;
    MyArray<float>    weigths;
};
using ClustersArray = MyArray<ClusterInfo>;

struct MyFbxMeshModel {
    FbxNode*            root;
    MyArray<FbxNode*>   nodes;
    MyArray<FbxMesh*>   meshes;
};


static FbxVector4 MetroVecToFbxVec(const vec3& v) {
    return FbxVector4(v.x, v.y, v.z);
}

static FbxVector4 MetroRotToFbxRot(const quat& q) {
#if 0
    FbxQuaternion fbxQuat(q.x, q.y, q.z, q.w);
    return fbxQuat.DecomposeSphericalXYZ();
#else
    vec3 euler = QuatToEuler(q);
    return FbxVector4(Rad2Deg(euler.x), Rad2Deg(euler.y), Rad2Deg(euler.z));
#endif
}


FbxSurfacePhong* FBXE_CreateMaterial(FbxManager* mgr, const CharString& textureName, const fs::path& texturesFolder, const CharString& extension) {
    MetroSurfaceDescription surfaceSet = MetroContext::Get().GetTexturesDB().GetSurfaceSetFromName(textureName, false);
    const CharString& albedoName = surfaceSet.albedo;
    const CharString& normalmapName = surfaceSet.normalmap;

    CharString albedoTextureName = fs::path(albedoName).filename().u8string();
    CharString textureFileName = albedoTextureName + extension;
    CharString texturePath = (texturesFolder / textureFileName).u8string();

    FbxFileTexture* texture = FbxFileTexture::Create(mgr, albedoTextureName.c_str());
    texture->SetFileName(texturePath.c_str());
    texture->SetTextureUse(FbxTexture::eStandard);
    texture->SetMappingType(FbxTexture::eUV);
    texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    texture->UVSwap = false;
    texture->SetTranslation(0.0, 0.0);
    texture->SetScale(1.0, 1.0);
    texture->SetRotation(0.0, 0.0);
    texture->SetAlphaSource(FbxTexture::eBlack);

    FbxFileTexture* normalmap = nullptr;
    if (!normalmapName.empty()) {
        CharString normalmapTextureName = fs::path(normalmapName).filename().u8string();
        CharString normalmapFileName = normalmapTextureName + extension;
        CharString normalmapPath = (texturesFolder / normalmapFileName).u8string();

        normalmap = FbxFileTexture::Create(mgr, normalmapTextureName.c_str());
        normalmap->SetFileName(normalmapPath.c_str());
        normalmap->SetTextureUse(FbxTexture::eBumpNormalMap);
        normalmap->SetMappingType(FbxTexture::eUV);
        normalmap->SetMaterialUse(FbxFileTexture::eModelMaterial);
        normalmap->UVSwap = false;
        normalmap->SetTranslation(0.0, 0.0);
        normalmap->SetScale(1.0, 1.0);
        normalmap->SetRotation(0.0, 0.0);
    }

    FbxSurfacePhong* material = FbxSurfacePhong::Create(mgr, textureName.c_str());
    material->Emissive = FbxDouble3(0.0, 0.0, 0.0);
    material->Diffuse.ConnectSrcObject(texture);
    material->Specular = FbxDouble3(1.0, 1.0, 1.0);
    material->SpecularFactor = 0.0;
    material->Shininess = 0.0; // simple diffuse

    if (normalmap) {
        material->NormalMap.ConnectSrcObject(normalmap);
    }

    return material;
}

static void FBXE_CreateMeshModel(FbxManager* mgr,
                                 FbxScene* scene,
                                 const MetroModelBase& model,
                                 const CharString& name,
                                 MyFbxMeshModel& fbxModel,
                                 MyFbxMaterialsDict& materialsDict,
                                 const fs::path& texturesFolder,
                                 const CharString& texExtension,
                                 const bool excludeCollision) {

    MyArray<MetroModelGeomData> gds;
    model.CollectGeomData(gds);

    fbxModel.root = FbxNode::Create(scene, name.c_str());

    size_t meshIdx = 0;
    for (auto& gd : gds) {
        MyArray<MetroVertex> vertices = MakeCommonVertices(gd);

        CharString meshName = CharString("mesh_") + std::to_string(meshIdx++);

        FbxMesh* fbxMesh = FbxMesh::Create(scene, meshName.c_str());

        // assign vertices
        fbxMesh->InitControlPoints(scast<int>(vertices.size()));
        FbxVector4* ptrCtrlPoints = fbxMesh->GetControlPoints();

        FbxGeometryElementNormal* normalElement = fbxMesh->CreateElementNormal();
        normalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
        normalElement->SetReferenceMode(FbxGeometryElement::eDirect);

        FbxGeometryElementUV* uvElement = fbxMesh->CreateElementUV("uv0");
        uvElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
        uvElement->SetReferenceMode(FbxGeometryElement::eDirect);

        for (const MetroVertex& v : vertices) {
            *ptrCtrlPoints = MetroVecToFbxVec(v.pos);
            normalElement->GetDirectArray().Add(MetroVecToFbxVec(v.normal));
            uvElement->GetDirectArray().Add(FbxVector2(v.uv0.x, 1.0 - v.uv0.y));

            ++ptrCtrlPoints;
        }

        FbxGeometryElementMaterial* materialElement = fbxMesh->CreateElementMaterial();
        materialElement->SetMappingMode(FbxGeometryElement::eAllSame);
        materialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
        materialElement->GetIndexArray().Add(0);

        // build polygons
        const MetroFace* faces = scast<const MetroFace*>(gd.faces);
        for (size_t i = 0; i < gd.mesh->facesCount; ++i) {
            fbxMesh->BeginPolygon();
            fbxMesh->AddPolygon(scast<int>(faces[i].c));
            fbxMesh->AddPolygon(scast<int>(faces[i].b));
            fbxMesh->AddPolygon(scast<int>(faces[i].a));
            fbxMesh->EndPolygon();
        }

        FbxNode* meshNode = FbxNode::Create(scene, meshName.c_str());
        meshNode->SetNodeAttribute(fbxMesh);

        FbxSurfacePhong* material = nullptr;
        HashString textureName = gd.texture;
        auto it = materialsDict.find(textureName);
        if (it != materialsDict.end()) {
            material = it->second;
        } else {
            material = FBXE_CreateMaterial(mgr, textureName.str, texturesFolder, texExtension);
            materialsDict[textureName] = material;
        }

        meshNode->SetShadingMode(FbxNode::eTextureShading);
        meshNode->AddMaterial(material);

        fbxModel.root->AddChild(meshNode);
        fbxModel.meshes.push_back(fbxMesh);
        fbxModel.nodes.push_back(meshNode);
    }
}

FbxNode* FBXE_InstantiateModel(FbxScene* scene, const CharString& name, const MyFbxMeshModel& fbxModel) {
    FbxNode* result = FbxNode::Create(scene, name.c_str());
    
    for (size_t i = 0; i < fbxModel.meshes.size(); ++i) {
        FbxMesh* srcMesh = fbxModel.meshes[i];
        FbxNode* srcNode = fbxModel.nodes[i];

        FbxNode* meshNode = FbxNode::Create(scene, srcNode->GetNameOnly());
        meshNode->SetNodeAttribute(srcMesh);
        meshNode->SetShadingMode(srcNode->GetShadingMode());
        meshNode->AddMaterial(srcNode->GetMaterial(0));
        result->AddChild(meshNode);
    }

    return result;
}

//#NOTE_SK: just because fucking FBX doesn't allow us to use quaternions for rotations
//          we'll get angles "clicking" issues
//          this is the only way I found to fight this issue
static void CorrectAnimTrackInterpolation(MyArray<FbxNode*>& boneNodes, FbxAnimLayer* animLayer) {
    for (FbxNode* bone : boneNodes) {
        FbxAnimCurveNode* rotCurveNode = bone->LclRotation.GetCurveNode(animLayer);
        if (rotCurveNode) {
            FbxAnimCurveFilterUnroll unrollFilter;
            unrollFilter.SetForceAutoTangents(true);
            unrollFilter.Apply(*rotCurveNode);
        }
    }
}

static void FBXE_CollectClusters(const MyArray<MetroVertex>& vertices, const MetroSkeleton* skeleton, ClustersArray& clusters) {
    const size_t numBones = skeleton->GetNumBones();
    clusters.resize(numBones);

    for (size_t i = 0; i < numBones; ++i) {
        ClusterInfo& cluster = clusters[i];

        for (size_t j = 0; j < vertices.size(); ++j) {
            const MetroVertex& v = vertices[j];

            for (size_t k = 0; k < 4; ++k) {
                const size_t boneIdx = v.bones[k];
                if (boneIdx == i && v.weights[k]) {
                    cluster.vertexIdxs.push_back(scast<int>(j));
                    cluster.weigths.push_back(scast<float>(v.weights[k]) * (1.0f / 255.0f));
                }
            }
        }
    }
}

static FbxNode* FBXE_CreateFBXSkeleton(FbxScene* scene, const MetroSkeleton* skeleton, MyArray<FbxNode*>& boneNodes) {
    const size_t numBones = skeleton->GetNumAttachPoints();
    boneNodes.reserve(numBones);

    for (size_t i = 0; i < numBones; ++i) {
        const size_t parentIdx = skeleton->GetBoneParentIdx(i);
        const CharString& name = skeleton->GetBoneName(i);

        FbxSkeleton* attribute = FbxSkeleton::Create(scene, name.c_str());
        if (kInvalidValue == parentIdx) {
            attribute->SetSkeletonType(FbxSkeleton::eRoot);
        } else {
            attribute->SetSkeletonType(FbxSkeleton::eLimbNode);
        }

        FbxNode* node = FbxNode::Create(scene, name.c_str());
        node->SetNodeAttribute(attribute);

        boneNodes.push_back(node);
    }

    FbxNode* rootNode = nullptr;
    for (size_t i = 0; i < numBones; ++i) {
        FbxNode* node = boneNodes[i];
        const size_t parentIdx = skeleton->GetBoneParentIdx(i);

        const quat& bindQ = skeleton->GetBoneRotation(i);
        const vec3& bindT = skeleton->GetBonePosition(i);

        node->LclTranslation.Set(MetroVecToFbxVec(bindT));
        node->LclRotation.Set(MetroRotToFbxRot(bindQ));

        if (kInvalidValue != parentIdx) {
            boneNodes[parentIdx]->AddChild(node);
        } else {
            rootNode = node;
        }
    }

    return rootNode;
}

static void FBXE_AddAnimTrackToScene(FbxScene* scene, const MetroMotion* motion, const CharString& animName, MyArray<FbxNode*>& skelNodes) {
    FbxAnimStack* animStack = FbxAnimStack::Create(scene, animName.c_str());
    FbxAnimLayer* animLayer = FbxAnimLayer::Create(scene->GetFbxManager(), "Base_Layer");
    animStack->AddMember(animLayer);

    FbxTime startTime, stopTime;
    startTime.SetGlobalTimeMode(FbxTime::eFrames30);
    stopTime.SetGlobalTimeMode(FbxTime::eFrames30);

    startTime.SetSecondDouble(0.0);

    // kinda hack to get animation duration
    const double animDuration = scast<double>(motion->GetMotionTimeInSeconds());
    stopTime.SetSecondDouble(animDuration);

    FbxTimeSpan animTimeSpan;
    animTimeSpan.Set(startTime, stopTime);
    animStack->SetLocalTimeSpan(animTimeSpan);

    FbxTime keyTime;
    int keyIndex;

    for (size_t i = 0; i < skelNodes.size(); ++i) {
        FbxNode* boneNode = skelNodes[i];

        if (motion->IsBoneAnimated(i)) {
            const auto& posCurve = motion->mBonesPositions[i];
            const auto& rotCurve = motion->mBonesRotations[i];

            boneNode->LclRotation.GetCurveNode(animLayer, true);
            boneNode->LclTranslation.GetCurveNode(animLayer, true);

            FbxAnimCurve* offsetCurve[3] = {
                boneNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, true),
                boneNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, true),
                boneNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, true)
            };

            offsetCurve[0]->KeyModifyBegin();
            offsetCurve[1]->KeyModifyBegin();
            offsetCurve[2]->KeyModifyBegin();

            if (!posCurve.points.empty()) {
                if (posCurve.points.size() == 1) {
                    FbxVector4 fv = MetroVecToFbxVec(vec3(posCurve.points.front().value));

                    keyTime.SetSecondDouble(0.0);

                    for (int k = 0; k < 3; ++k) {
                        keyIndex = offsetCurve[k]->KeyAdd(keyTime);
                        offsetCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                        offsetCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationLinear);
                    }
                } else {
                    for (auto& pt : posCurve.points) {
                        FbxVector4 fv = MetroVecToFbxVec(vec3(pt.value));

                        keyTime.SetSecondDouble(scast<double>(pt.time));

                        for (int k = 0; k < 3; ++k) {
                            keyIndex = offsetCurve[k]->KeyAdd(keyTime);
                            offsetCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                            offsetCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationCubic);
                        }
                    }
                }
            }

            offsetCurve[0]->KeyModifyEnd();
            offsetCurve[1]->KeyModifyEnd();
            offsetCurve[2]->KeyModifyEnd();

            FbxAnimCurve* rotationCurve[3] = {
                boneNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, true),
                boneNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, true),
                boneNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, true)
            };

            rotationCurve[0]->KeyModifyBegin();
            rotationCurve[1]->KeyModifyBegin();
            rotationCurve[2]->KeyModifyBegin();

            if (!rotCurve.points.empty()) {
                if (rotCurve.points.size() == 1) {
                    FbxVector4 fv = MetroRotToFbxRot(*rcast<const quat*>(&rotCurve.points.front().value));

                    keyTime.SetSecondDouble(0.0);

                    for (int k = 0; k < 3; ++k) {
                        keyIndex = rotationCurve[k]->KeyAdd(keyTime);
                        rotationCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                        rotationCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationLinear);
                    }
                } else {
                    for (auto& pt : rotCurve.points) {
                        FbxVector4 fv = MetroRotToFbxRot(*rcast<const quat*>(&pt.value));

                        keyTime.SetSecondDouble(scast<double>(pt.time));

                        for (int k = 0; k < 3; ++k) {
                            keyIndex = rotationCurve[k]->KeyAdd(keyTime);
                            rotationCurve[k]->KeySetValue(keyIndex, scast<float>(fv[k]));
                            rotationCurve[k]->KeySetInterpolation(keyIndex, FbxAnimCurveDef::eInterpolationCubic);
                        }
                    }
                }
            }

            rotationCurve[0]->KeyModifyEnd();
            rotationCurve[1]->KeyModifyEnd();
            rotationCurve[2]->KeyModifyEnd();
        }
    }

    CorrectAnimTrackInterpolation(skelNodes, animLayer);
}

static bool FBXE_SaveFBXScene(FbxManager* mgr, FbxScene* scene, FbxIOSettings* ios, const fs::path& path, const bool saveMesh, const bool saveAnim) {
    // now export all this
    FbxGlobalSettings& settings = scene->GetGlobalSettings();
    settings.SetAxisSystem(FbxAxisSystem(FbxAxisSystem::eMayaYUp));
    settings.SetOriginalUpAxis(FbxAxisSystem(FbxAxisSystem::eMayaYUp));
    settings.SetSystemUnit(FbxSystemUnit::m);

    // export
    FbxExporter* exp = FbxExporter::Create(mgr, "");
    const int format = mgr->GetIOPluginRegistry()->GetNativeWriterFormat();

    exp->SetFileExportVersion(FBX_2011_00_COMPATIBLE);

    ios->SetBoolProp(EXP_FBX_MATERIAL, saveMesh);
    ios->SetBoolProp(EXP_FBX_TEXTURE, saveMesh);
    ios->SetBoolProp(EXP_FBX_EMBEDDED, false);
    ios->SetBoolProp(EXP_FBX_SHAPE, saveMesh);
    ios->SetBoolProp(EXP_FBX_GOBO, saveMesh);
    ios->SetBoolProp(EXP_FBX_MODEL, saveMesh);
    ios->SetBoolProp(EXP_FBX_ANIMATION, saveAnim);
    ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    if (exp->Initialize(path.u8string().c_str(), format, ios)) {
        if (!exp->Export(scene)) {
            exp->Destroy(true);
            return false;
        }
    }

    exp->Destroy(true);

    return true;
}




ExporterFBX::ExporterFBX()
    : mExcludeCollision(false)
    , mExportMesh(true)
    , mExportSkeleton(false)
    , mExportAnimation(false)
    , mExportMotionIdx(kInvalidValue)
{
    mExporterName = "MetroEX";
}
ExporterFBX::~ExporterFBX()
{
}

void ExporterFBX::SetExporterName(const CharString& exporterName) {
    mExporterName = exporterName;
}

void ExporterFBX::SetTexturesExtension(const CharString& ext) {
    mTexturesExtension = ext;
}

void ExporterFBX::SetExcludeCollision(const bool b) {
    mExcludeCollision = b;
}

void ExporterFBX::SetExportMesh(const bool b) {
    mExportMesh = b;
}

void ExporterFBX::SetExportSkeleton(const bool b) {
    mExportSkeleton = b;
}

void ExporterFBX::SetExportAnimation(const bool b) {
    mExportAnimation = b;
}

void ExporterFBX::SetExportMotionIdx(const size_t idx) {
    mExportMotionIdx = idx;
}

bool ExporterFBX::ExportModel(const MetroModelBase& model, const fs::path& filePath) {
    FbxManager* mgr = FbxManager::Create();
    if (!mgr) {
        return false;
    }

    mTexturesFolder = filePath.parent_path();
    CharString modelName = filePath.stem().string();

    FbxIOSettings* ios = FbxIOSettings::Create(mgr, IOSROOT);
    mgr->SetIOSettings(ios);

    FbxScene* scene = FbxScene::Create(mgr, "Metro model");
    FbxDocumentInfo* info = scene->GetSceneInfo();
    if (info) {
        info->Original_ApplicationVendor = FbxString("iOrange");
        info->Original_ApplicationName = FbxString(mExporterName.data());
        info->mTitle = FbxString("Metro model");
        info->mComment = FbxString(CharString("Exported using ").append(mExporterName).append(" created by iOrange").data());
    }

    MyFbxMeshModel fbxMeshModel;
    MyFbxMaterialsDict fbxMaterials;
    if (mExportMesh) {
        FBXE_CreateMeshModel(mgr, scene, model, modelName, fbxMeshModel, fbxMaterials, mTexturesFolder, mTexturesExtension, mExcludeCollision);
        scene->GetRootNode()->AddChild(fbxMeshModel.root);
    }

    MyArray<FbxNode*> boneNodes;
    if (model.IsSkeleton() && (mExportSkeleton || mExportAnimation)) {
        const MetroModelSkeleton& skelModel = scast<const MetroModelSkeleton&>(model);
        RefPtr<MetroSkeleton> skeleton = skelModel.GetSkeleton();
        if (skeleton) {
            FbxNode* rootBoneNode = FBXE_CreateFBXSkeleton(scene, skeleton.get(), boneNodes);
            scene->GetRootNode()->AddChild(rootBoneNode);

            FbxPose* bindPose = FbxPose::Create(scene, "BindPose");
            bindPose->SetIsBindPose(true);

            for (FbxNode* node : boneNodes) {
                bindPose->Add(node, node->EvaluateGlobalTransform());
            }

            MyArray<MetroModelGeomData> gds;
            skelModel.CollectGeomData(gds);

            size_t meshIdx = 0;
            for (auto& gd : gds) {
                MyArray<MetroVertex> vertices = MakeCommonVertices(gd);

                MyArray<ClusterInfo> clusters;
                FBXE_CollectClusters(vertices, skeleton.get(), clusters);

                FbxSkin* skin = FbxSkin::Create(scene, "");
                for (size_t i = 0; i < clusters.size(); ++i) {
                    const ClusterInfo& cluster = clusters[i];
                    if (!cluster.vertexIdxs.empty()) {
                        FbxCluster* fbxCluster = FbxCluster::Create(scene, "");
                        FbxNode* linkNode = boneNodes[i];
                        fbxCluster->SetLink(linkNode);
                        fbxCluster->SetLinkMode(FbxCluster::eTotalOne);
                        for (size_t j = 0; j < cluster.vertexIdxs.size(); ++j) {
                            fbxCluster->AddControlPointIndex(cluster.vertexIdxs[j], cluster.weigths[j]);
                        }
                        //fbxCluster->SetTransformMatrix(meshXMatrix);
                        fbxCluster->SetTransformLinkMatrix(linkNode->EvaluateGlobalTransform());
                        skin->AddCluster(fbxCluster);
                    }
                }

                if (mExportMesh) {
                    FbxMesh* fbxMesh = fbxMeshModel.meshes[meshIdx];
                    FbxNode* meshNode = fbxMeshModel.nodes[meshIdx];
                    FbxAMatrix meshXMatrix = meshNode->EvaluateGlobalTransform();
                    fbxMesh->AddDeformer(skin);
                    bindPose->Add(meshNode, meshXMatrix);
                }

                ++meshIdx;
            }

            scene->AddPose(bindPose);
        }
    }

    if (mExportAnimation) {
        const MetroModelSkeleton& skelModel = scast<const MetroModelSkeleton&>(model);
        RefPtr<MetroSkeleton> skeleton = skelModel.GetSkeleton();
        if (skeleton) {
            if (mExportMotionIdx != kInvalidValue) {
                RefPtr<MetroMotion> motion = skeleton->GetMotion(mExportMotionIdx);
                FBXE_AddAnimTrackToScene(scene, motion.get(), motion->GetName(), boneNodes);
            } else {
                for (size_t i = 0; i < skeleton->GetNumMotions(); ++i) {
                    RefPtr<MetroMotion> motion = skeleton->GetMotion(i);
                    FBXE_AddAnimTrackToScene(scene, motion.get(), motion->GetName(), boneNodes);
                }
            }
        }
    }

    const bool result = FBXE_SaveFBXScene(mgr, scene, ios, filePath, mExportMesh, mExportSkeleton || mExportAnimation);

    mgr->Destroy();

    if (result) {
        mUsedTextures.resize(0);
        for (auto& it : fbxMaterials) {
            mUsedTextures.push_back(it.first.str);
        }
    }

    return result;
}

bool ExporterFBX::ExportLevel(const MetroLevel& level, const fs::path& filePath) {
    FbxManager* mgr = FbxManager::Create();
    if (!mgr) {
        return false;
    }

    fs::path levelFolder = filePath.parent_path();
    mTexturesFolder = levelFolder / "textures";

    fs::create_directories(mTexturesFolder);

    FbxIOSettings* ios = FbxIOSettings::Create(mgr, IOSROOT);
    mgr->SetIOSettings(ios);

    FbxScene* scene = FbxScene::Create(mgr, "Metro level");
    FbxDocumentInfo* info = scene->GetSceneInfo();
    if (info) {
        info->Original_ApplicationVendor = FbxString("iOrange");
        info->Original_ApplicationName = FbxString("MetroEX");
        info->mTitle = FbxString("Metro level");
        info->mComment = FbxString("Exported using MetroEX created by iOrange");
    }


    MyFbxMaterialsDict fbxMaterials;

    const size_t numSectors = level.GetNumSectors();
#if 0
    // pre-create materials
    for (size_t i = 0; i < numSectors; ++i) {
        const LevelSector& sector = level.GetSector(i);
        for (const auto& ss : sector.sections) {
            if (!ss.textureName.empty()) {
                const HashString textureName = ss.textureName;
                auto it = fbxMaterials.find(textureName);
                if (it == fbxMaterials.end()) {
                    FbxSurfacePhong* material = FBXE_CreateMaterial(mgr, textureName.str, mTexturesFolder, mTexturesExtension);
                    fbxMaterials[textureName] = material;
                }
            }
        }
    }

    // export level geo
    for (size_t i = 0; i < numSectors; ++i) {
        const LevelSector& sector = level.GetSector(i);
        FbxNode* sectorNode = FbxNode::Create(scene, sector.name.c_str());
        scene->GetRootNode()->AddChild(sectorNode);

        const size_t numSectorSections = sector.sections.size();
        for (size_t j = 0; j < numSectorSections; ++j) {
            const SectorSection& ss = sector.sections[j];

            // empty section ???
            if (!ss.desc.numVertices || !ss.desc.numIndices) {
                continue;
            }

            CharString sectionName = sector.name + "_section_" + std::to_string(j);
            FbxMesh* fbxMesh = FbxMesh::Create(scene, sectionName.c_str());

            // assign vertices
            fbxMesh->InitControlPoints(scast<int>(ss.desc.numVertices));
            FbxVector4* ptrCtrlPoints = fbxMesh->GetControlPoints();

            FbxGeometryElementNormal* normalElement = fbxMesh->CreateElementNormal();
            normalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
            normalElement->SetReferenceMode(FbxGeometryElement::eDirect);

            FbxGeometryElementUV* uv0Element = fbxMesh->CreateElementUV("uv0");
            uv0Element->SetMappingMode(FbxGeometryElement::eByControlPoint);
            uv0Element->SetReferenceMode(FbxGeometryElement::eDirect);

            FbxGeometryElementUV* uv1Element = fbxMesh->CreateElementUV("uv1");
            uv1Element->SetMappingMode(FbxGeometryElement::eByControlPoint);
            uv1Element->SetReferenceMode(FbxGeometryElement::eDirect);

            // build vertices
            for (size_t k = 0; k < ss.desc.numVertices; ++k) {
                const MetroVertex& v = ConvertVertex(sector.vertices[ss.desc.vbOffset + k]);

                *ptrCtrlPoints = MetroVecToFbxVec(v.pos);
                normalElement->GetDirectArray().Add(MetroVecToFbxVec(v.normal));
                uv0Element->GetDirectArray().Add(FbxVector2(v.uv0.x, 1.0f - v.uv0.y));
                uv1Element->GetDirectArray().Add(FbxVector2(v.uv1.x, 1.0f - v.uv1.y));

                ++ptrCtrlPoints;
            }

            FbxGeometryElementMaterial* materialElement = fbxMesh->CreateElementMaterial();
            materialElement->SetMappingMode(FbxGeometryElement::eAllSame);
            materialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
            materialElement->GetIndexArray().Add(0);

            // build polygons
            for (size_t k = 0; k < ss.desc.numIndices; k += 3) {
                const uint16_t a = sector.indices[ss.desc.ibOffset + k + 0];
                const uint16_t b = sector.indices[ss.desc.ibOffset + k + 1];
                const uint16_t c = sector.indices[ss.desc.ibOffset + k + 2];

                fbxMesh->BeginPolygon();
                fbxMesh->AddPolygon(scast<int>(c));
                fbxMesh->AddPolygon(scast<int>(b));
                fbxMesh->AddPolygon(scast<int>(a));
                fbxMesh->EndPolygon();
            }

            FbxNode* meshNode = FbxNode::Create(scene, sectionName.c_str());
            meshNode->SetNodeAttribute(fbxMesh);
            sectorNode->AddChild(meshNode);

            const CharString& textureName = ss.textureName;
            if (!textureName.empty()) {
                auto it = fbxMaterials.find(textureName);
                if (it != fbxMaterials.end()) {
                    meshNode->SetShadingMode(FbxNode::eTextureShading);
                    meshNode->AddMaterial(it->second);
                }
            }
        }
    }
#endif
    // export entities
    MyDict<HashString, MyFbxMeshModel> fbxModelsCache;

    const size_t numEntities = level.GetNumEntities();
    for (size_t i = 0; i < numEntities; ++i) {
        const CharString& visual = level.GetEntityVisual(i);
        if (!visual.empty()) {
            const CharString& entityName = level.GetEntityName(i);

            FbxNode* modelNode = nullptr;
            HashString hashName = visual;
            auto it = fbxModelsCache.find(hashName);
            if (it != fbxModelsCache.end()) {
                modelNode = FBXE_InstantiateModel(scene, entityName, it->second);
            } else {
                const uint32_t loadFlags = MetroModelLoadParams::LoadGeometry | MetroModelLoadParams::LoadTPresets;
                RefPtr<MetroModelBase> mdl = MetroModelFactory::CreateModelFromFullName(visual, loadFlags);
                if (mdl) {
                    MyFbxMeshModel newFbxModel;
                    FBXE_CreateMeshModel(mgr, scene, *mdl, entityName, newFbxModel, fbxMaterials, mTexturesFolder, mTexturesExtension, mExcludeCollision);
                    fbxModelsCache[hashName] = newFbxModel;
                    modelNode = newFbxModel.root;
                }
            }

            if (modelNode) {
                mat4 pose = MatFromPose(level.GetEntityTransform(i));
                vec3 pos, scale;
                quat rot;
                MatDecompose(pose, pos, scale, rot);

                pos = MetroSwizzle(pos);
                scale = MetroSwizzle(scale);
                rot = MetroSwizzle(rot);

                modelNode->LclTranslation.Set(MetroVecToFbxVec(pos));
                modelNode->LclRotation.Set(MetroRotToFbxRot(rot));
                modelNode->LclScaling.Set(MetroVecToFbxVec(scale));

                scene->GetRootNode()->AddChild(modelNode);
            }
        }
    }

    const bool result = FBXE_SaveFBXScene(mgr, scene, ios, filePath, true, false);

    mgr->Destroy();

    if (result) {
        mUsedTextures.resize(0);
        for (auto& it : fbxMaterials) {
            mUsedTextures.push_back(it.first.str);
        }
    }

    return result;
}

const StringArray& ExporterFBX::GetUsedTextures() const {
    return mUsedTextures;
}

const fs::path& ExporterFBX::GetTexturesFolder() const {
    return mTexturesFolder;
}
