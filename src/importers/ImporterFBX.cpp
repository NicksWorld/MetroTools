#include "ImporterFBX.h"

#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroModel.h"

#define FBXSDK_NEW_API 1
#define FBXSDK_SHARED 1
#define FBXSDK_NAMESPACE_USING 0
#include "fbxsdk.h"

#pragma comment(lib, "libfbxsdk.lib")

static const CharString kShadowSuffix   = "_shadow";
static const CharString kLOD1Suffix     = "_lod1";
static const CharString kLOD2Suffix     = "_lod2";

//
//static bool IsNodeABone(FbxNode* node) {
//    //return node->GetSkeleton() != nullptr;
//    //return true;
//    CharString name = node->GetNameOnly();
//    return name._Starts_with("bip");
//}
//
//void FlattenBones(FbxNode* node, FlatFbxNode& flatNode, size_t nodeIndex, size_t parentIndex, MyArray<FlatFbxNode>& flattenedArray) {
//    flatNode.fbxNode = node;
//    flatNode.name = node->GetNameOnly();
//    flatNode.parentIdx = parentIndex;
//
//    const int numChildren = node->GetChildCount();
//    if (numChildren) {
//        const size_t firstChildIndex = flattenedArray.size();
//        flattenedArray.reserve(flattenedArray.size() + scast<size_t>(numChildren));
//
//        for (int i = 0; i < numChildren; ++i) {
//            const size_t childIndex = firstChildIndex + scast<size_t>(i);
//            FbxNode* childNode = node->GetChild(i);
//            if (IsNodeABone(childNode)) {
//                flattenedArray.push_back({});
//                FlattenBones(childNode, flattenedArray[childIndex], childIndex, nodeIndex, flattenedArray);
//            }
//        }
//    }
//}

struct ControlPoint {
    fbxsdk::FbxVector4 pos;
    MyArray<ImporterFBX::UniversalVertex::BoneInfluence> influences;
};

static fbxsdk::FbxAMatrix GetGeometryTransformation(fbxsdk::FbxNode* fbxNode) {
    const fbxsdk::FbxVector4 t = fbxNode->GetGeometricTranslation(fbxsdk::FbxNode::eSourcePivot);
    const fbxsdk::FbxVector4 r = fbxNode->GetGeometricRotation(fbxsdk::FbxNode::eSourcePivot);
    const fbxsdk::FbxVector4 s = fbxNode->GetGeometricScaling(fbxsdk::FbxNode::eSourcePivot);

    return fbxsdk::FbxAMatrix(t, r, s);
}

static fbxsdk::FbxAMatrix GetFixedWorldTransform(fbxsdk::FbxNode* fbxNode) {
    static fbxsdk::FbxAMatrix sMirrorZAxis(fbxsdk::FbxVector4(0.0, 0.0,  0.0),
                                           fbxsdk::FbxVector4(0.0, 0.0,  0.0),
                                           fbxsdk::FbxVector4(1.0, 1.0, -1.0));

    fbxsdk::FbxAMatrix globalTransform = fbxNode->EvaluateGlobalTransform();
    fbxsdk::FbxAMatrix geometryTransform = GetGeometryTransformation(fbxNode);
    fbxsdk::FbxAMatrix worldTransform = globalTransform * geometryTransform;
    return sMirrorZAxis * worldTransform * sMirrorZAxis;
}


vec3 FbxVecToVec3(const fbxsdk::FbxVector4& v) {
    return vec3(scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
vec3 FbxDbl3ToVec3(const fbxsdk::FbxDouble3& v) {
    return vec3(scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
quat FbxQuatToQuat(const fbxsdk::FbxQuaternion& v) {
    return quat(scast<float>(v[3]), scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
mat4 FbxMatToMat(const fbxsdk::FbxAMatrix& m) {
    mat4 result;

    const double* ptrD = m;
    float* ptrF = MatToPtrMutable(result);

    for (size_t i = 0; i < 16; ++i) {
        ptrF[i] = scast<float>(ptrD[i]);
    }

    return result;
}


static fbxsdk::FbxVector4 MetroVecToFbxVec(const vec3& v) {
    return fbxsdk::FbxVector4(v.x, v.y, v.z);
}

static fbxsdk::FbxVector4 MetroRotToFbxRot(const quat& q) {
    vec3 euler = QuatToEuler(q);
    return fbxsdk::FbxVector4(Rad2Deg(euler.x), Rad2Deg(euler.y), Rad2Deg(euler.z));
}


template <typename T>
static fbxsdk::FbxVector4 GetNormalFromElement(T* element, const int controlPointIdx, const int rawVertexIdx) {
    fbxsdk::FbxVector4 normal;

    switch (element->GetMappingMode()) {
        case fbxsdk::FbxGeometryElement::eByControlPoint: {
            switch (element->GetReferenceMode()) {
                case fbxsdk::FbxGeometryElement::eDirect: {
                    normal = element->GetDirectArray().GetAt(controlPointIdx);
                } break;

                case fbxsdk::FbxGeometryElement::eIndexToDirect: {
                    const int normalIdx = element->GetIndexArray().GetAt(controlPointIdx);
                    normal = element->GetDirectArray().GetAt(normalIdx);
                } break;

                default:
                    assert(false);
            }
        } break;

        case fbxsdk::FbxGeometryElement::eByPolygonVertex: {
            switch (element->GetReferenceMode()) {
                case fbxsdk::FbxGeometryElement::eDirect: {
                    normal = element->GetDirectArray().GetAt(rawVertexIdx);
                } break;

                case fbxsdk::FbxGeometryElement::eIndexToDirect: {
                    const int normalIdx = element->GetIndexArray().GetAt(rawVertexIdx);
                    normal = element->GetDirectArray().GetAt(normalIdx);
                } break;

                default:
                    assert(false);
            }
        } break;

        default:
            assert(false);
    }

    return normal;
}

template <typename T>
struct VerticesCollector {
    MyArray<T>        vertices;
    MyArray<uint16_t> indices;

    void AddVertex(const T& v) {
        const auto begin = this->vertices.begin();
        const auto end = this->vertices.end();
        const auto it = std::find(begin, end, v);

        if (it == end) {
            indices.push_back(scast<uint16_t>(this->vertices.size()));
            this->vertices.push_back(v);
        } else {
            const size_t idx = std::distance(begin, it);
            this->indices.push_back(scast<uint16_t>(idx));
        }
    }
};


ImporterFBX::ImporterFBX()
    : mGameVersion(MetroGameVersion::Redux)
    , mAnimStartTime(0.0)
    , mAnimEndTime(0.0)
{
}
ImporterFBX::~ImporterFBX() {

}


void ImporterFBX::SetGameVersion(const MetroGameVersion gameVersion) {
    mGameVersion = gameVersion;
}

void ImporterFBX::SetSkeleton(const fs::path& path) {
    MemStream stream = OSReadFile(path);
    if (stream) {
        mSkeleton = MakeRefPtr<MetroSkeleton>();
        if (!mSkeleton->LoadFromData(stream)) {
            mSkeleton = nullptr;
        }
    }
}

bool ImporterFBX::ImportAnimation(const fs::path& path, MetroMotion& motion) {
    bool result = false;

    if (!mSkeleton) {
        return result;
    }

    fbxsdk::FbxManager* fbxMgr = fbxsdk::FbxManager::Create();
    if (!fbxMgr) {
        return result;
    }

    fbxsdk::FbxIOSettings* ios = fbxsdk::FbxIOSettings::Create(fbxMgr, IOSROOT);
    fbxMgr->SetIOSettings(ios);

    fbxsdk::FbxScene* fbxScene = fbxsdk::FbxScene::Create(fbxMgr, "");
    if (fbxScene) {
        fbxsdk::FbxImporter* fbxImporter = fbxsdk::FbxImporter::Create(fbxMgr, "");

        ios->SetBoolProp(IMP_FBX_TEXTURE, false);
        ios->SetBoolProp(IMP_FBX_ANIMATION, true);
        ios->SetBoolProp(IMP_FBX_MODEL, false);
        ios->SetBoolProp(IMP_META_DATA, true);

        const bool success = fbxImporter->Initialize(path.u8string().data(), -1, ios);
        if (success && fbxImporter->IsFBX()) {
            if (fbxImporter->Import(fbxScene)) {
                const fbxsdk::FbxAxisSystem ourAxis = fbxsdk::FbxAxisSystem::MayaYUp;

                fbxsdk::FbxAxisSystem fileAxis = fbxScene->GetGlobalSettings().GetAxisSystem();
                if (fileAxis != ourAxis) {
                    ourAxis.ConvertScene(fbxScene);
                }

                const int numAnimStacks = fbxScene->GetSrcObjectCount<fbxsdk::FbxAnimStack>();
                if (numAnimStacks) {
                    //FbxTakeInfo* takeInfo = fbxImporter->GetTakeInfo(0);
                    //FbxAnimStack* animStack = scast<FbxAnimStack*>(fbxScene->RootProperty.FindSrcObject(FbxCriteria::ObjectType(fbxMgr->FindClass("FbxAnimStack")), takeInfo->mName, 0));
                    fbxsdk::FbxAnimStack* animStack = fbxScene->GetSrcObject<fbxsdk::FbxAnimStack>(0);

                    fbxScene->SetCurrentAnimationStack(animStack);

                    //FbxAnimLayer* animLayer = scast<FbxAnimLayer*>(animStack->RootProperty.GetSrcObject(FbxCriteria::ObjectType(fbxMgr->FindClass("FbxAnimLayer")), 0));
                    fbxsdk::FbxAnimLayer* animLayer = animStack->GetMember<fbxsdk::FbxAnimLayer>(0);

                    fbxsdk::FbxNode* rootNode = fbxScene->GetRootNode();

                    this->CollectFbxBones(rootNode);

                    std::sort(mFbxBones.begin(), mFbxBones.end(), [](const auto& a, const auto& b)->bool {
                        return a.skeletonAttpIdx < b.skeletonAttpIdx;
                    });
                    std::sort(mFbxLocators.begin(), mFbxLocators.end(), [](const auto& a, const auto& b)->bool {
                        return a.skeletonAttpIdx < b.skeletonAttpIdx;
                    });

                    //mBonesCurves.resize(mFbxBones.size());
                    motion.mBonesPositions.resize(mFbxBones.size());
                    motion.mBonesRotations.resize(mFbxBones.size());
                    motion.mBonesScales.resize(mFbxBones.size());

                    mAnimStartTime = animStack->LocalStart.Get().GetSecondDouble();
                    mAnimEndTime = animStack->LocalStop.Get().GetSecondDouble();
                    //const double animLength = endTime - startTime;

                    const size_t numAnimFrames = animStack->GetLocalTimeSpan().GetDuration().GetFrameCount() + 1;

                    fbxsdk::FbxTime startTime = animStack->LocalStart.Get();

#if 0
                    size_t boneIdx = 0;
                    for (BoneFbxNode& bfn : mFbxBones) {
                        FbxNode* node = bfn.fbxNode;

                        this->GatherRotCurve(animLayer, node, mBonesCurves[boneIdx].rotCurve, startTime, numAnimFrames);
                        this->GatherPosCurve(animLayer, node, mBonesCurves[boneIdx].posCurve, startTime, numAnimFrames);

                        ++boneIdx;
                    }
#else
                    fbxsdk::FbxTime curTime = startTime;
                    fbxsdk::FbxTime period;
                    period.SetTime(0, 0, 0, 1, 0);

                    for (size_t frame = 0; frame < numAnimFrames; ++frame) {

                        size_t boneIdx = 0;
                        for (BoneFbxNode& bfn : mFbxBones) {
                            fbxsdk::FbxNode* node = bfn.fbxNode;

                            AttributeCurve& rotCurve = motion.mBonesRotations[boneIdx];
                            AttributeCurve& posCurve = motion.mBonesPositions[boneIdx];

                            fbxsdk::FbxAMatrix transform = node->EvaluateLocalTransform(curTime);

                            //FbxAMatrix transform = node->EvaluateGlobalTransform(curTime);
                            //if (bfn.skeletonAttpParentIdx != kInvalidValue) {
                            //    BoneFbxNode& pbfn = mFbxBones[bfn.skeletonAttpParentIdx];
                            //    FbxAMatrix parentTransform = pbfn.fbxNode->EvaluateGlobalTransform();

                            //    transform = transform * parentTransform.Inverse();
                            //}

                            const quat& qBind = mSkeleton->GetBoneRotation(boneIdx);
                            const vec3& tBind = mSkeleton->GetBonePosition(boneIdx);

#if 1
                            quat q = FbxQuatToQuat(transform.GetQ());
                            vec3 t = FbxVecToVec3(transform.GetT());

                            //q *= QuatConjugate(qBind);
                            //t -= tBind;
#else
                            FbxAMatrix bindMatrix;
                            bindMatrix.SetR(MetroRotToFbxRot(qBind));
                            bindMatrix.SetT(MetroVecToFbxVec(tBind));

                            FbxAMatrix localDeltaMove = transform * bindMatrix.Inverse();

                            quat q = FbxQuatToQuat(localDeltaMove.GetQ());
                            vec3 t = FbxVecToVec3(localDeltaMove.GetT());
#endif

                            const float time = scast<float>(curTime.GetSecondDouble());

                            if (LengthSqr(q) > MM_Epsilon) {
                                const vec4& qv4 = *rcast<const vec4*>(&q);
                                if (rotCurve.points.empty() || rotCurve.points.back().value != qv4) {
                                    rotCurve.points.push_back({ time, qv4 });
                                }
                            }

                            if (LengthSqr(t) > MM_Epsilon) {
                                const vec4 tv4 = vec4(t, 0.0f);
                                if (posCurve.points.empty() || posCurve.points.back().value != tv4) {
                                    posCurve.points.push_back({ time, tv4 });
                                }
                            }

                            ++boneIdx;
                        }

                        curTime += period;
                    }

                    // header
                    motion.mVersion = 14;   // Redux ???
                    motion.mBonesCRC = mSkeleton->GetBonesCRC();   // ok ???
                    motion.mNumBones = mFbxBones.size();
                    motion.mNumLocators = 0;
                    // info
                    motion.mFlags = 0x4002;//0x4100;
                    motion.mSpeed = 1.0f;
                    motion.mAccrue = 2.0f;
                    motion.mFalloff = 2.0f;
                    motion.mNumFrames = numAnimFrames;
                    motion.mJumpFrame = 0;
                    motion.mLandFrame = numAnimFrames - 1;

                    motion.mAffectedBones.Clear();
                    motion.mHighQualityBones.Clear();
                    motion.mMotionDataHeader.bonesMask.Clear();
                    for (size_t i = 0; i < mFbxBones.size(); ++i) {
                        motion.mAffectedBones.SetBit(i, true);
                        motion.mHighQualityBones.SetBit(i, true);
                        motion.mMotionDataHeader.bonesMask.SetBit(i, true);
                    }

                    motion.mMotionDataHeader.numLocators = 0;
                    motion.mMotionDataHeader.numXforms = 0;
                    motion.mMotionDataHeader.unknown_0 = 0x3F8000003F800000;
#endif
                }
            }
        }

        fbxImporter->Destroy();
        fbxScene->Destroy();
    }

    fbxMgr->Destroy();

    return result;
}

// https://yejunny.wordpress.com/2020/02/24/import-fbx-animation-using-c/
// https://github.com/lang1991/FBXExporter/tree/master/FBXExporter
// IMPORTANT !!!! - https://blender.stackexchange.com/questions/89975/blender-adds-extra-bones-to-ends-of-armature-when-i-export-as-fbx-or-obj
// https://ozz-animation.docsforge.com/0.12.1/framework/tools/fbx2mesh.cc/
RefPtr<MetroModelBase> ImporterFBX::ImportModel(const fs::path& filePath) {
    RefPtr<MetroModelBase> result = nullptr;

    fbxsdk::FbxManager* fbxMgr = fbxsdk::FbxManager::Create();
    if (!fbxMgr) {
        return result;
    }

    fbxsdk::FbxIOSettings* ios = fbxsdk::FbxIOSettings::Create(fbxMgr, IOSROOT);
    fbxMgr->SetIOSettings(ios);

    fbxsdk::FbxScene* fbxScene = fbxsdk::FbxScene::Create(fbxMgr, "");
    if (fbxScene) {
        fbxsdk::FbxImporter* fbxImporter = fbxsdk::FbxImporter::Create(fbxMgr, "");

        ios->SetBoolProp(IMP_FBX_TEXTURE, false);
        ios->SetBoolProp(IMP_FBX_ANIMATION, true);
        ios->SetBoolProp(IMP_FBX_MODEL, true);
        ios->SetBoolProp(IMP_META_DATA, true);

        const bool success = fbxImporter->Initialize(filePath.u8string().c_str(), -1, ios);
        if (success && fbxImporter->IsFBX()) {
            if (fbxImporter->Import(fbxScene)) {
                fbxsdk::FbxAxisSystem fileAxis = fbxScene->GetGlobalSettings().GetAxisSystem();
                const fbxsdk::FbxAxisSystem workingAxis(fbxsdk::FbxAxisSystem::eYAxis, fbxsdk::FbxAxisSystem::eParityOdd, fbxsdk::FbxAxisSystem::eRightHanded);
                if (fileAxis != workingAxis) {
                    workingAxis.ConvertScene(fbxScene);
                }

                if (fbxsdk::FbxSystemUnit::m != fbxScene->GetGlobalSettings().GetSystemUnit()) {
                    fbxsdk::FbxSystemUnit::m.ConvertScene(fbxScene);
                }

                FbxMeshesFounder foundMeshes;
                fbxsdk::FbxNode* rootNode = fbxScene->GetRootNode();
                this->CollectSceneMeshesRecursive(rootNode, foundMeshes);

                //// try to find the bind pose
                //FbxPose* fbxBindPose = nullptr;
                //for (int i = 0; i < fbxScene->GetPoseCount(); ++i) {
                //    FbxPose* fbxPose = fbxScene->GetPose(i);
                //    if (fbxPose->IsBindPose()) {
                //        fbxBindPose = fbxPose;
                //        break;
                //    }
                //}

                mSkeleton = this->TryToImportSkeleton(rootNode);
                if (mSkeleton != nullptr) {
                    RefPtr<MetroModelSkeleton> modelSkeleton = MakeRefPtr<MetroModelSkeleton>();
                    modelSkeleton->SetSkeleton(mSkeleton);
                    result = modelSkeleton;
                } else {
                    result = MakeRefPtr<MetroModelHierarchy>();
                }

                result->SetModelVersionBasedOnGameVersion(mGameVersion);

                const bool hasShadowGeometry = (foundMeshes.shadowMeshes.size() == foundMeshes.meshes.size());

                for (size_t i = 0, numMeshes = foundMeshes.meshes.size(); i < numMeshes; ++i) {
                    this->AddMeshToModel(result, foundMeshes.nodes[i], foundMeshes.meshes[i], hasShadowGeometry ? foundMeshes.shadowMeshes[i] : nullptr);
                }

                if (mSkeleton == nullptr) {
                    if (!foundMeshes.lod1Meshes.empty()) {
                        RefPtr<MetroModelBase> lod1Model = MakeRefPtr<MetroModelHierarchy>();
                        for (size_t i = 0, numMeshes = foundMeshes.lod1Meshes.size(); i < numMeshes; ++i) {
                            this->AddMeshToModel(lod1Model, foundMeshes.lod1Nodes[i], foundMeshes.lod1Meshes[i], nullptr);
                        }
                        // add LOD1
                        SCastRefPtr<MetroModelHierarchy>(result)->AddLOD(lod1Model);

                        if (!foundMeshes.lod2Meshes.empty()) {
                            RefPtr<MetroModelBase> lod2Model = MakeRefPtr<MetroModelHierarchy>();
                            for (size_t i = 0, numMeshes = foundMeshes.lod2Meshes.size(); i < numMeshes; ++i) {
                                this->AddMeshToModel(lod2Model, foundMeshes.lod2Nodes[i], foundMeshes.lod2Meshes[i], nullptr);
                            }
                            // add LOD2
                            SCastRefPtr<MetroModelHierarchy>(result)->AddLOD(lod2Model);
                        }
                    }
                }
            }
        }

        fbxImporter->Destroy();
        fbxScene->Destroy();
    }

    fbxMgr->Destroy();

    return result;
}

// returns 0 if not a lod mesh, returns 1 if LOD1 mesh, returns 2 if LOD2 mesh
static int FBXI_IsLODMesh(fbxsdk::FbxNode* node) {
    fbxsdk::FbxNode* parent = node->GetParent();
    if (parent) {
        CharString parentName = parent->GetNameOnly().Buffer();

        if (StrEndsWith(parentName, kLOD1Suffix)) {
            return 1;
        } else if (StrEndsWith(parentName, kLOD2Suffix)) {
            return 2;
        }
    }

    return 0;
}

void ImporterFBX::CollectSceneMeshesRecursive(fbxsdk::FbxNode* rootNode, FbxMeshesFounder& foundMeshes) {
    if (rootNode->GetNodeAttribute() && rootNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::eMesh) {
        fbxsdk::FbxMesh* mesh = scast<fbxsdk::FbxMesh*>(rootNode->GetNodeAttribute());
        CharString nodeName = rootNode->GetNameOnly().Buffer();
        if (StrEndsWith(nodeName, kShadowSuffix)) {
            foundMeshes.shadowMeshes.push_back(mesh);
        } else {
            const int lodType = FBXI_IsLODMesh(rootNode);
            if (1 == lodType) {
                foundMeshes.lod1Nodes.push_back(rootNode);
                foundMeshes.lod1Meshes.push_back(mesh);
            } else if (2 == lodType) {
                foundMeshes.lod2Nodes.push_back(rootNode);
                foundMeshes.lod2Meshes.push_back(mesh);
            } else {
                foundMeshes.nodes.push_back(rootNode);
                foundMeshes.meshes.push_back(mesh);
            }
        }
    } else {
        for (int i = 0, numChild = rootNode->GetChildCount(); i < numChild; ++i) {
            this->CollectSceneMeshesRecursive(rootNode->GetChild(i), foundMeshes);
        }
    }
}

void ImporterFBX::AddMeshToModel(RefPtr<MetroModelBase>& model, fbxsdk::FbxNode* fbxNode, fbxsdk::FbxMesh* fbxMesh, fbxsdk::FbxMesh* fbxShadowMesh) {
    fbxsdk::FbxAMatrix meshXMatrix = GetFixedWorldTransform(fbxNode);
    quat meshRotation = FbxQuatToQuat(meshXMatrix.GetQ());

    const int numControlPoints = fbxMesh->GetControlPointsCount();
    const fbxsdk::FbxVector4* controlPoints = fbxMesh->GetControlPoints();

    static fbxsdk::FbxVector4 sFlipVec(1.0, 1.0, -1.0, 1.0);

    // collect control points (yes, FBX is weird)
    MyArray<ControlPoint> myControlPoints(numControlPoints);
    for (int i = 0; i < numControlPoints; ++i) {
        ControlPoint& ctrlPt = myControlPoints[i];
        ctrlPt.pos = meshXMatrix.MultT(controlPoints[i] * sFlipVec);
    }

    MyArray<vec3> myShadowPoints;
    if (fbxShadowMesh) {
        const int numShadowPoints = fbxShadowMesh->GetControlPointsCount();
        const fbxsdk::FbxVector4* shadowPoints = fbxShadowMesh->GetControlPoints();
        myShadowPoints.resize(numShadowPoints);
        for (int i = 0; i < numShadowPoints; ++i) {
            myShadowPoints[i] = FbxVecToVec3(meshXMatrix.MultT(shadowPoints[i] * sFlipVec));
        }
    }

    // if we have skeleton - collect control points influences
    if (mSkeleton) {
        const int numDeformers = fbxMesh->GetDeformerCount();
        for (int i = 0; i < numDeformers; ++i) {
            fbxsdk::FbxDeformer* fbxDeformer = fbxMesh->GetDeformer(i);
            if (fbxDeformer->GetDeformerType() == fbxsdk::FbxDeformer::eSkin) {
                fbxsdk::FbxSkin* fbxSkin = scast<fbxsdk::FbxSkin*>(fbxDeformer);

                const int numClusters = fbxSkin->GetClusterCount();
                for (int j = 0; j < numClusters; ++j) {
                    fbxsdk::FbxCluster* fbxCluster = fbxSkin->GetCluster(j);
                    fbxsdk::FbxNode* fbxClusterLink = fbxCluster->GetLink();
                    if (fbxClusterLink) {
                        CharString linkName = fbxClusterLink->GetNameOnly().Buffer();
                        const size_t actualBoneIdx = mSkeleton->FindBone(linkName);
                        assert(actualBoneIdx != kInvalidValue);

                        const int numClusterVertices = fbxCluster->GetControlPointIndicesCount();
                        if (numClusterVertices > 0) {
                            const int* indices = fbxCluster->GetControlPointIndices();
                            const double* weights = fbxCluster->GetControlPointWeights();

                            for (int k = 0; k < numClusterVertices; ++k) {
                                if (weights[k] > 0.0) {
                                    UniversalVertex::BoneInfluence influence;
                                    influence.idx = scast<uint32_t>(actualBoneIdx);
                                    influence.weight = scast<float>(weights[k]);

                                    myControlPoints[indices[k]].influences.push_back(influence);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    fbxsdk::FbxGeometryElementNormal* fbxNormals = fbxMesh->GetElementNormal();
    if (!fbxNormals) {
        fbxMesh->GenerateNormals(false, false, false);
        fbxNormals = fbxMesh->GetElementNormal();
    }

    fbxsdk::FbxGeometryElementTangent* fbxTangents = fbxMesh->GetElementTangent();
    fbxsdk::FbxGeometryElementBinormal* fbxBitangents = fbxMesh->GetElementBinormal();
    fbxsdk::FbxGeometryElementUV* fbxUV = fbxMesh->GetElementUV();

    assert(fbxNormals);
    assert(fbxUV);

    if (!fbxTangents) {
        fbxMesh->GenerateTangentsData(nullptr, false);
        fbxTangents = fbxMesh->GetElementTangent();
        fbxBitangents = fbxMesh->GetElementBinormal();
    }

    const int polygonCount = fbxMesh->GetPolygonCount();

    MyArray<UniversalVertex> vertices;

    AABBox bbox;
    bbox.Reset();
    int vertexIdx = 0;
    for (int i = 0; i < polygonCount; ++i) {
        assert(fbxMesh->GetPolygonSize(i) == 3);

        for (int j = 0; j < 3; ++j) {
            const int idx = fbxMesh->GetPolygonVertex(i, j);
            vec3 pos = FbxVecToVec3(myControlPoints[idx].pos);

            UniversalVertex vertex;
            vertex.pos = pos;

            fbxsdk::FbxVector4 normal = GetNormalFromElement(fbxNormals, idx, vertexIdx) * sFlipVec;
            vertex.normal = Normalize(QuatRotate(meshRotation, FbxVecToVec3(normal)));

            if (fbxTangents) {
                fbxsdk::FbxVector4 tangent = GetNormalFromElement(fbxTangents, idx, vertexIdx) * sFlipVec;
                vertex.tangent = Normalize(QuatRotate(meshRotation, FbxVecToVec3(tangent)));
            }

            if (fbxBitangents) {
                fbxsdk::FbxVector4 bitangent = GetNormalFromElement(fbxBitangents, idx, vertexIdx) * sFlipVec;
                vertex.bitangent = Normalize(QuatRotate(meshRotation, FbxVecToVec3(bitangent)));
            }

            fbxsdk::FbxVector2 uv;
            switch (fbxUV->GetMappingMode()) {
                case fbxsdk::FbxGeometryElement::eByControlPoint: {
                    switch (fbxUV->GetReferenceMode()) {
                        case fbxsdk::FbxGeometryElement::eDirect: {
                            uv = fbxUV->GetDirectArray().GetAt(idx);
                        } break;

                        case fbxsdk::FbxGeometryElement::eIndexToDirect: {
                            const int uvIdx = fbxUV->GetIndexArray().GetAt(idx);
                            uv = fbxUV->GetDirectArray().GetAt(uvIdx);
                        } break;

                        default:
                            assert(false);
                    }
                } break;

                case fbxsdk::FbxGeometryElement::eByPolygonVertex: {
                    const int uvIdx = fbxMesh->GetTextureUVIndex(i, j);

                    switch (fbxUV->GetReferenceMode()) {
                        case fbxsdk::FbxGeometryElement::eDirect:
                        case fbxsdk::FbxGeometryElement::eIndexToDirect: {
                            uv = fbxUV->GetDirectArray().GetAt(uvIdx);
                        } break;

                        default:
                            assert(false);
                    }
                } break;

                default:
                    assert(false);
            }
            vertex.uv.x = scast<float>(uv[0]);
            vertex.uv.y = scast<float>(1.0 - uv[1]);

            vertex.boneInfluences = myControlPoints[idx].influences;

            vertices.push_back(vertex);

            bbox.Absorb(pos);

            ++vertexIdx;
        }
    }

    myControlPoints.clear();

    VerticesCollector<UniversalVertex> collector;
    for (const UniversalVertex& v : vertices) {
        collector.AddVertex(v);
    }

    const size_t numVertices = collector.vertices.size();
    const size_t numFaces = collector.indices.size() / 3;
    // reverse winding because we're flipping verts above
    for (size_t f = 0; f < numFaces; ++f) {
        std::swap(collector.indices[f * 3 + 0], collector.indices[f * 3 + 2]);
    }

    VerticesCollector<vec3> shadowCollector;
    for (const vec3& v : myShadowPoints) {
        shadowCollector.AddVertex(v);
    }

    const size_t numShadowVertices = shadowCollector.vertices.size();
    const size_t numShadowFaces = shadowCollector.indices.size() / 3;
    // reverse winding because we're flipping verts above
    for (size_t f = 0; f < numShadowFaces; ++f) {
        std::swap(shadowCollector.indices[f * 3 + 0], shadowCollector.indices[f * 3 + 2]);
    }

    RefPtr<MetroModelBase> mesh;

    if (mSkeleton) {
        this->FixUpSkinnedVertices(collector.vertices);
        BytesArray bonesRemapTable =  this->BuildBonesRemapTable(collector.vertices);

        const float vscale = bbox.MaximumValue();
        const float invVScale = 1.0f / vscale;

        MyArray<VertexSkinned> skinnedVertices(numVertices);
        for (size_t i = 0; i < numVertices; ++i) {
            VertexSkinned& dst = skinnedVertices[i];
            const UniversalVertex& src = collector.vertices[i];

            EncodeSkinnedPosition(src.pos * invVScale, dst.pos);
            dst.normal = EncodeNormal(src.normal, 1.0f);
            dst.aux0 = EncodeNormal(src.tangent, 1.0f);
            dst.aux1 = EncodeNormal(src.bitangent, 1.0f);
            for (size_t i = 0; i < 4; ++i) {
                dst.bones[i] = scast<uint8_t>(Clamp(src.boneInfluences[i].idx * 3, 0u, 255u));
                dst.weights[i] = scast<uint8_t>(Clamp(src.boneInfluences[i].weight * 255.0f, 0.0f, 255.0f));
            }
            MetroSwizzle(dst.bones);
            MetroSwizzle(dst.weights);
            EncodeSkinnedUV(src.uv, dst.uv);
        }


        RefPtr<MetroModelSkin> skinnedMesh = MakeRefPtr<MetroModelSkin>();
        skinnedMesh->CreateMesh(numVertices, numFaces, vscale);
        skinnedMesh->CopyVerticesData(skinnedVertices.data());
        skinnedMesh->CopyFacesData(collector.indices.data());
        skinnedMesh->SetBonesRemapTable(bonesRemapTable);
        MetroOBB identityOBB;
        identityOBB.matrix = mat3(1.0f);
        identityOBB.hsize = vec3(0.1f, 0.1f, 0.1f);
        identityOBB.offset = vec3(0.0f, 0.0f, 0.0f);
        MyArray<MetroOBB> obbs(bonesRemapTable.size(), identityOBB);
        skinnedMesh->SetBonesOBB(obbs);

        mesh = skinnedMesh;
    } else {
        MyArray<VertexStatic> staticVertices(numVertices);
        for (size_t i = 0; i < numVertices; ++i) {
            VertexStatic& dst = staticVertices[i];
            const UniversalVertex& src = collector.vertices[i];

            dst.pos = src.pos;
            dst.normal = EncodeNormal(src.normal, 1.0f);
            dst.aux0 = EncodeNormal(src.tangent, 1.0f);
            dst.aux1 = EncodeNormal(src.bitangent, 1.0f);
            dst.uv = src.uv;
        }

        MyArray<VertexStaticShadow> shadowVertices(numShadowVertices);
        for (size_t i = 0; i < numShadowVertices; ++i) {
            VertexStaticShadow& dst = shadowVertices[i];
            dst.pos = shadowCollector.vertices[i];
            dst.padding = 0;
        }

        RefPtr<MetroModelStd> staticMesh = MakeRefPtr<MetroModelStd>();
        staticMesh->CreateMesh(numVertices, numFaces, numShadowVertices, numShadowFaces);
        staticMesh->CopyVerticesData(staticVertices.data());
        staticMesh->CopyFacesData(collector.indices.data());
        if (numShadowVertices) {
            staticMesh->CopyShadowVerticesData(shadowVertices.data());
            staticMesh->CopyShadowFacesData(shadowCollector.indices.data());
        }

        mesh = staticMesh;
    }

    mesh->SetMaterialString(kEmptyString, 0); // force-create material strings array
    CharString meshName = fbxMesh->GetNameOnly();
    mesh->SetMaterialString(meshName, 3);

    BSphere bsphere;
    bsphere.center = bbox.Center();
    bsphere.radius = Length(bbox.Extent());

    mesh->SetBBox(bbox);
    mesh->SetBSphere(bsphere);
    mesh->SetModelVersionBasedOnGameVersion(mGameVersion);

    if (mSkeleton) {
        SCastRefPtr<MetroModelSkeleton>(model)->AddChildEx(mesh);
    } else {
        SCastRefPtr<MetroModelHierarchy>(model)->AddChild(mesh);
    }
}

RefPtr<MetroSkeleton> ImporterFBX::TryToImportSkeleton(fbxsdk::FbxNode* fbxRootNode) {
    mJointsBones.clear();
    mJointsLocators.clear();

    for (int i = 0; i < fbxRootNode->GetChildCount(); ++i) {
        this->AddJointRecursive(fbxRootNode->GetChild(i), nullptr);
    }

    const size_t numBonesJoints = mJointsBones.size();
    if (!numBonesJoints) {
        return nullptr;
    }

    RefPtr<MetroSkeleton> skeleton = MakeRefPtr<MetroSkeleton>();

    MyArray<MetroBone> metroBones(numBonesJoints);
    for (size_t i = 0; i < numBonesJoints; ++i) {
        const JointFromFbx& joint = mJointsBones[i];
        MetroBone& metroBone = metroBones[i];

        metroBone.bp = 0;
        metroBone.bpf = 0;
        metroBone.name = joint.fbxNode->GetNameOnly().Buffer();
        metroBone.parent = joint.fbxParentNode ? joint.fbxParentNode->GetNameOnly().Buffer() : kEmptyString;

        //FbxAMatrix transform = joint.fbxNode->EvaluateLocalTransform();
        //metroBone.q = FbxQuatToQuat(transform.GetQ());
        //metroBone.t = FbxVecToVec3(transform.GetT());
        fbxsdk::FbxAMatrix localTransform;
        if (!joint.fbxParentNode) {
            localTransform = joint.fbxNode->EvaluateGlobalTransform();
        } else {
            fbxsdk::FbxAMatrix parentInvert = joint.fbxParentNode->EvaluateGlobalTransform().Inverse();
            localTransform = joint.fbxNode->EvaluateGlobalTransform() * parentInvert;
        }
        metroBone.q = FbxQuatToQuat(localTransform.GetQ());
        metroBone.t = FbxVecToVec3(localTransform.GetT());
    }
    skeleton->SetBones(metroBones);

    const size_t numLocatorsJoints = mJointsLocators.size();
    if (numLocatorsJoints > 0) {
        MyArray<MetroLocator> metroLocators(numLocatorsJoints);
        for (size_t i = 0; i < numLocatorsJoints; ++i) {
            const JointFromFbx& joint = mJointsLocators[i];
            MetroLocator& metroLocator = metroLocators[i];

            metroLocator.fl = 0;
            metroLocator.name = joint.fbxNode->GetNameOnly().Buffer();
            metroLocator.parent = joint.fbxParentNode ? joint.fbxParentNode->GetNameOnly().Buffer() : kEmptyString;

            fbxsdk::FbxAMatrix transform = joint.fbxNode->EvaluateLocalTransform();
            metroLocator.q = FbxQuatToQuat(transform.GetQ());
            metroLocator.t = FbxVecToVec3(transform.GetT());
        }
        skeleton->SetLocators(metroLocators);
    }

    return skeleton;

    //const int numPoseNodes = fbxPose->GetCount();
    //MyArray<FbxNode*> fbxBones;
    //FbxNode* fbxRootBone = nullptr;
    //MyArray<FbxNode*> fbxLocators;
    //for (int i = 0; i < numPoseNodes; ++i) {
    //    FbxNode* fbxNode = fbxPose->GetNode(i);
    //    if (fbxNode->GetNodeAttribute() && fbxNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
    //        CharString fbxNodeName = fbxNode->GetNameOnly().Buffer();
    //        if (StrStartsWith(fbxNodeName, "loc_")) {
    //            fbxLocators.push_back(fbxNode);
    //        } else {
    //            fbxBones.push_back(fbxNode);

    //            FbxSkeleton* fbxSkelAttrib = scast<FbxSkeleton*>(fbxNode->GetNodeAttribute());
    //            if (fbxSkelAttrib->GetSkeletonType() == FbxSkeleton::eRoot) {
    //                fbxRootBone = fbxNode;
    //            }
    //        }
    //    }
    //}

    //if (!fbxBones.empty()) {
    //    MyArray<MetroBone> bones; bones.reserve(fbxBones.size());
    //    
    //}
}

void ImporterFBX::AddJointRecursive(fbxsdk::FbxNode* fbxNode, fbxsdk::FbxNode* fbxParentNode) {
    if (fbxNode->GetNodeAttribute() && fbxNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::eSkeleton) {
        CharString fbxNodeName = fbxNode->GetNameOnly().Buffer();

        //#NOTE_SK: fucking Blender's armature >:(
        if (!StrEndsWith(fbxNodeName, "_end")) {
            JointFromFbx joint;
            if (fbxParentNode->GetNodeAttribute() && fbxParentNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::eSkeleton) {
                joint.fbxParentNode = fbxParentNode;
            } else {
                joint.fbxParentNode = nullptr;
            }
            joint.fbxNode = fbxNode;


            if (StrStartsWith(fbxNodeName, "loc_")) {
                mJointsLocators.push_back(joint);
            } else {
                mJointsBones.push_back(joint);
            }
        }
    }

    for (int i = 0; i < fbxNode->GetChildCount(); ++i) {
        this->AddJointRecursive(fbxNode->GetChild(i), fbxNode);
    }
}

void ImporterFBX::FixUpSkinnedVertices(MyArray<UniversalVertex>& vertices) {
    // cap vertex influences at 4 and re-normalize
    for (UniversalVertex& v : vertices) {
        // sort highest weight first
        std::sort(v.boneInfluences.begin(), v.boneInfluences.end(), [](const UniversalVertex::BoneInfluence& a, const UniversalVertex::BoneInfluence& b)->bool {
            return a.weight > b.weight;
        });
        if (v.boneInfluences.size() > 4) {
            v.boneInfluences.resize(4);
        }

        float sum = 0.0f;
        for (auto& infl : v.boneInfluences) {
            sum += infl.weight;
        }
        if (sum > 0.0f) {
            for (auto& infl : v.boneInfluences) {
                infl.weight /= sum;
            }
        }

        // if bones # is less than 4, add zero weights to keep other code simpler (always assumes we have 4)
        while (v.boneInfluences.size() < 4) {
            v.boneInfluences.push_back({ 0, 0.0f });
        }
    }
}

BytesArray ImporterFBX::BuildBonesRemapTable(MyArray<UniversalVertex>& vertices) {
    BytesArray remapTable;

    auto addToRemapTable = [&remapTable](const uint8_t v)->uint8_t {
        auto beg = remapTable.begin();
        auto end = remapTable.end();
        auto it = std::find(beg, end, v);
        if (it != end) {
            return scast<uint8_t>(std::distance(beg, it));
        } else {
            const uint8_t result = scast<uint8_t>(remapTable.size());
            remapTable.push_back(v);
            return result;
        }
    };

    for (UniversalVertex& v : vertices) {
        for (auto& infl : v.boneInfluences) {
            if (infl.weight > 0.0f) {
                infl.idx = addToRemapTable(scast<uint8_t>(infl.idx));
            }
        }
    }

    return remapTable;
}

void ImporterFBX::CollectFbxBones(fbxsdk::FbxNode* node) {
    BoneFbxNode boneNode;
    boneNode.fbxNode = node;
    boneNode.name = node->GetNameOnly();
    boneNode.skeletonAttpIdx = mSkeleton->FindBone(boneNode.name);

    if (boneNode.skeletonAttpIdx != kInvalidValue) {
        boneNode.skeletonAttpParentIdx = mSkeleton->GetBoneParentIdx(boneNode.skeletonAttpIdx);

        if (mSkeleton->IsAttachPointABone(boneNode.skeletonAttpIdx)) {
            mFbxBones.emplace_back(boneNode);
        } else if (mSkeleton->IsAttachPointALocator(boneNode.skeletonAttpIdx)) {
            mFbxLocators.emplace_back(boneNode);
        }
    }

    const int numChildren = node->GetChildCount();
    for (int i = 0; i < numChildren; ++i) {
        this->CollectFbxBones(node->GetChild(i));
    }
}

void ImporterFBX::GatherRotCurve(fbxsdk::FbxAnimLayer* animLayer, fbxsdk::FbxNode* node, AnimCurve::RotCurve& rotCurve, const fbxsdk::FbxTime& startTime, const size_t numFrames) {
    fbxsdk::FbxAnimCurve* curves[3] = { nullptr };

    curves[0] = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, false);
    curves[1] = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
    curves[2] = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

    if (curves[0] || curves[1] || curves[2]) {
        fbxsdk::FbxTime curTime = startTime;
        fbxsdk::FbxTime period;
        period.SetTime(0, 0, 0, 1, 0);

        for (size_t frame = 0; frame < numFrames; ++frame) {
            vec3 euler(0.0f);

            for (size_t i = 0; i < std::size(curves); ++i) {
                if (curves[i]) {
                    curves[i]->Evaluate(curTime);
                    euler[i] = curves[i]->GetValue();
                }
            }

            const float time = scast<float>(curTime.GetSecondDouble());
            quat q = QuatFromEuler(euler);

            if (frame && rotCurve.points.back().value == q) {
                continue;
            }

            rotCurve.points.push_back({ time, q });

            curTime += period;
        }
    }
}

void ImporterFBX::GatherPosCurve(fbxsdk::FbxAnimLayer* animLayer, fbxsdk::FbxNode* node, AnimCurve::PosCurve& posCurve, const fbxsdk::FbxTime& startTime, const size_t numFrames) {
    fbxsdk::FbxAnimCurve* curves[3] = { nullptr };

    curves[0] = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, false);
    curves[1] = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
    curves[2] = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

    if (curves[0] || curves[1] || curves[2]) {
        fbxsdk::FbxTime curTime = startTime;
        fbxsdk::FbxTime period;
        period.SetTime(0, 0, 0, 1, 0);

        for (size_t frame = 0; frame < numFrames; ++frame) {
            vec3 pos(0.0f);

            for (size_t i = 0; i < std::size(curves); ++i) {
                if (curves[i]) {
                    curves[i]->Evaluate(curTime);
                    pos[i] = curves[i]->GetValue();
                }
            }

            if (frame && posCurve.points.back().value == pos) {
                continue;
            }

            const float time = scast<float>(curTime.GetSecondDouble());
            posCurve.points.push_back({ time, pos });

            curTime += period;
        }
    }
}
