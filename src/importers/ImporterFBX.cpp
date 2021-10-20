#include "ImporterFBX.h"

#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroModel.h"

#define FBXSDK_NEW_API
#define FBXSDK_SHARED
#include "fbxsdk.h"

#pragma comment(lib, "libfbxsdk.lib")

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
    FbxVector4 pos;
    MyArray<ImporterFBX::UniversalVertex::BoneInfluence> influences;
};

static FbxAMatrix GetGeometryTransformation(FbxNode* fbxNode) {
    const FbxVector4 t = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 r = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 s = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(r, r, s);
}


vec3 FbxVecToVec3(const FbxVector4& v) {
    return vec3(scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
vec3 FbxDbl3ToVec3(const FbxDouble3& v) {
    return vec3(scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
quat FbxQuatToQuat(const FbxQuaternion& v) {
    return quat(scast<float>(v[3]), scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
mat4 FbxMatToMat(const FbxAMatrix& m) {
    mat4 result;

    const double* ptrD = m;
    float* ptrF = MatToPtrMutable(result);

    for (size_t i = 0; i < 16; ++i) {
        ptrF[i] = scast<float>(ptrD[i]);
    }

    return result;
}


static FbxVector4 MetroVecToFbxVec(const vec3& v) {
    return FbxVector4(v.x, v.y, v.z);
}

static FbxVector4 MetroRotToFbxRot(const quat& q) {
    vec3 euler = QuatToEuler(q);
    return FbxVector4(Rad2Deg(euler.x), Rad2Deg(euler.y), Rad2Deg(euler.z));
}


template <typename T>
static FbxVector4 GetNormalFromElement(T* element, const int controlPointIdx, const int rawVertexIdx) {
    FbxVector4 normal;

    switch (element->GetMappingMode()) {
        case FbxGeometryElement::eByControlPoint: {
            switch (element->GetReferenceMode()) {
                case FbxGeometryElement::eDirect: {
                    normal = element->GetDirectArray().GetAt(controlPointIdx);
                } break;

                case FbxGeometryElement::eIndexToDirect: {
                    const int normalIdx = element->GetIndexArray().GetAt(controlPointIdx);
                    normal = element->GetDirectArray().GetAt(normalIdx);
                } break;

                default:
                    assert(false);
            }
        } break;

        case FbxGeometryElement::eByPolygonVertex: {
            switch (element->GetReferenceMode()) {
                case FbxGeometryElement::eDirect: {
                    normal = element->GetDirectArray().GetAt(rawVertexIdx);
                } break;

                case FbxGeometryElement::eIndexToDirect: {
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

struct VerticesCollector {
    MyArray<ImporterFBX::UniversalVertex> vertices;
    MyArray<uint16_t> indices;

    void AddVertex(const ImporterFBX::UniversalVertex& v) {
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

    FbxManager* fbxMgr = FbxManager::Create();
    if (!fbxMgr) {
        return result;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(fbxMgr, IOSROOT);
    fbxMgr->SetIOSettings(ios);

    FbxScene* fbxScene = FbxScene::Create(fbxMgr, "");
    if (fbxScene) {
        FbxImporter* fbxImporter = FbxImporter::Create(fbxMgr, "");

        ios->SetBoolProp(IMP_FBX_TEXTURE, false);
        ios->SetBoolProp(IMP_FBX_ANIMATION, true);
        ios->SetBoolProp(IMP_FBX_MODEL, false);
        ios->SetBoolProp(IMP_META_DATA, true);

        const bool success = fbxImporter->Initialize(path.u8string().data(), -1, ios);
        if (success && fbxImporter->IsFBX()) {
            if (fbxImporter->Import(fbxScene)) {
                const FbxAxisSystem ourAxis = FbxAxisSystem::MayaYUp;

                FbxAxisSystem fileAxis = fbxScene->GetGlobalSettings().GetAxisSystem();
                if (fileAxis != ourAxis) {
                    ourAxis.ConvertScene(fbxScene);
                }

                const int numAnimStacks = fbxScene->GetSrcObjectCount<FbxAnimStack>();
                if (numAnimStacks) {
                    //FbxTakeInfo* takeInfo = fbxImporter->GetTakeInfo(0);
                    //FbxAnimStack* animStack = scast<FbxAnimStack*>(fbxScene->RootProperty.FindSrcObject(FbxCriteria::ObjectType(fbxMgr->FindClass("FbxAnimStack")), takeInfo->mName, 0));
                    FbxAnimStack* animStack = fbxScene->GetSrcObject<FbxAnimStack>(0);

                    fbxScene->SetCurrentAnimationStack(animStack);

                    //FbxAnimLayer* animLayer = scast<FbxAnimLayer*>(animStack->RootProperty.GetSrcObject(FbxCriteria::ObjectType(fbxMgr->FindClass("FbxAnimLayer")), 0));
                    FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);

                    FbxNode* rootNode = fbxScene->GetRootNode();

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

                    FbxTime startTime = animStack->LocalStart.Get();

#if 0
                    size_t boneIdx = 0;
                    for (BoneFbxNode& bfn : mFbxBones) {
                        FbxNode* node = bfn.fbxNode;

                        this->GatherRotCurve(animLayer, node, mBonesCurves[boneIdx].rotCurve, startTime, numAnimFrames);
                        this->GatherPosCurve(animLayer, node, mBonesCurves[boneIdx].posCurve, startTime, numAnimFrames);

                        ++boneIdx;
                    }
#else
                    FbxTime curTime = startTime;
                    FbxTime period;
                    period.SetTime(0, 0, 0, 1, 0);

                    for (size_t frame = 0; frame < numAnimFrames; ++frame) {

                        size_t boneIdx = 0;
                        for (BoneFbxNode& bfn : mFbxBones) {
                            FbxNode* node = bfn.fbxNode;

                            AttributeCurve& rotCurve = motion.mBonesRotations[boneIdx];
                            AttributeCurve& posCurve = motion.mBonesPositions[boneIdx];

                            FbxAMatrix transform = node->EvaluateLocalTransform(curTime);

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

    FbxManager* fbxMgr = FbxManager::Create();
    if (!fbxMgr) {
        return result;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(fbxMgr, IOSROOT);
    fbxMgr->SetIOSettings(ios);

    FbxScene* fbxScene = FbxScene::Create(fbxMgr, "");
    if (fbxScene) {
        FbxImporter* fbxImporter = FbxImporter::Create(fbxMgr, "");

        ios->SetBoolProp(IMP_FBX_TEXTURE, false);
        ios->SetBoolProp(IMP_FBX_ANIMATION, true);
        ios->SetBoolProp(IMP_FBX_MODEL, true);
        ios->SetBoolProp(IMP_META_DATA, true);

        const bool success = fbxImporter->Initialize(filePath.u8string().c_str(), -1, ios);
        if (success && fbxImporter->IsFBX()) {
            if (fbxImporter->Import(fbxScene)) {
                const FbxAxisSystem ourAxis = FbxAxisSystem::MayaYUp;

                FbxAxisSystem fileAxis = fbxScene->GetGlobalSettings().GetAxisSystem();
                if (fileAxis != ourAxis) {
                    ourAxis.ConvertScene(fbxScene);
                }

                MyArray<FbxMesh*> fbxMeshesList;

                FbxNode* rootNode = fbxScene->GetRootNode();
                for (int i = 0; i < rootNode->GetChildCount(); ++i) {
                    FbxNode* node = rootNode->GetChild(i);
                    if (node && node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
                        fbxMeshesList.push_back(scast<FbxMesh*>(node->GetNodeAttribute()));
                    }
                }

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

                result->SetModelVersionBasedOnGameVersion(mGameVersion); //! Redux

                for (FbxMesh* fbxMesh : fbxMeshesList) {
                    this->AddMeshToModel(result, fbxMesh);
                }
            }
        }

        fbxImporter->Destroy();
        fbxScene->Destroy();
    }

    fbxMgr->Destroy();

    return result;
}

void ImporterFBX::AddMeshToModel(RefPtr<MetroModelBase>& model, fbxsdk::FbxMesh* fbxMesh) {
    const int numControlPoints = fbxMesh->GetControlPointsCount();
    const FbxVector4* controlPoints = fbxMesh->GetControlPoints();

    // collect control points (yes, FBX is weird)
    MyArray<ControlPoint> myControlPoints(numControlPoints);
    for (int i = 0; i < numControlPoints; ++i) {
        ControlPoint& ctrlPt = myControlPoints[i];
        ctrlPt.pos = controlPoints[i];
    }

    // if we have skeleton - collect control points influences
    if (mSkeleton) {
        const int numDeformers = fbxMesh->GetDeformerCount();
        for (int i = 0; i < numDeformers; ++i) {
            FbxDeformer* fbxDeformer = fbxMesh->GetDeformer(i);
            if (fbxDeformer->GetDeformerType() == FbxDeformer::eSkin) {
                FbxSkin* fbxSkin = scast<FbxSkin*>(fbxDeformer);

                const int numClusters = fbxSkin->GetClusterCount();
                for (int j = 0; j < numClusters; ++j) {
                    FbxCluster* fbxCluster = fbxSkin->GetCluster(j);
                    FbxNode* fbxClusterLink = fbxCluster->GetLink();
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

            FbxVector4 normal = GetNormalFromElement(fbxNormals, idx, vertexIdx);
            vertex.normal = FbxVecToVec3(normal);

            if (fbxTangents) {
                FbxVector4 tangent = GetNormalFromElement(fbxTangents, idx, vertexIdx);
                vertex.tangent = FbxVecToVec3(tangent);
            }

            if (fbxBitangents) {
                FbxVector4 bitangent = GetNormalFromElement(fbxBitangents, idx, vertexIdx);
                vertex.bitangent = FbxVecToVec3(bitangent);
            }

            FbxVector2 uv;
            switch (fbxUV->GetMappingMode()) {
                case FbxGeometryElement::eByControlPoint: {
                    switch (fbxUV->GetReferenceMode()) {
                        case FbxGeometryElement::eDirect: {
                            uv = fbxUV->GetDirectArray().GetAt(idx);
                        } break;

                        case FbxGeometryElement::eIndexToDirect: {
                            const int uvIdx = fbxUV->GetIndexArray().GetAt(idx);
                            uv = fbxUV->GetDirectArray().GetAt(uvIdx);
                        } break;

                        default:
                            assert(false);
                    }
                } break;

                case FbxGeometryElement::eByPolygonVertex: {
                    const int uvIdx = fbxMesh->GetTextureUVIndex(i, j);

                    switch (fbxUV->GetReferenceMode()) {
                        case FbxGeometryElement::eDirect:
                        case FbxGeometryElement::eIndexToDirect: {
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

    VerticesCollector collector;
    for (const UniversalVertex& v : vertices) {
        collector.AddVertex(v);
    }

    const size_t numVertices = collector.vertices.size();
    const size_t numFaces = collector.indices.size() / 3;

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

        RefPtr<MetroModelStd> staticMesh = MakeRefPtr<MetroModelStd>();
        staticMesh->CreateMesh(numVertices, numFaces);
        staticMesh->CopyVerticesData(staticVertices.data());
        staticMesh->CopyFacesData(collector.indices.data());

        mesh = staticMesh;
    }

    mesh->SetMaterialString(kEmptyString, 0); // force-create material strings array

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

        FbxAMatrix transform = joint.fbxNode->EvaluateLocalTransform();
        metroBone.q = FbxQuatToQuat(transform.GetQ());
        metroBone.t = FbxVecToVec3(transform.GetT());
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

            FbxAMatrix transform = joint.fbxNode->EvaluateLocalTransform();
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
    if (fbxNode->GetNodeAttribute() && fbxNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
        CharString fbxNodeName = fbxNode->GetNameOnly().Buffer();

        //#NOTE_SK: fucking Blender's armature >:(
        if (!StrEndsWith(fbxNodeName, "_end")) {
            JointFromFbx joint;
            if (fbxParentNode->GetNodeAttribute() && fbxParentNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
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
    FbxAnimCurve* curves[3] = { nullptr };

    curves[0] = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, false);
    curves[1] = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
    curves[2] = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

    if (curves[0] || curves[1] || curves[2]) {
        FbxTime curTime = startTime;
        FbxTime period;
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
    FbxAnimCurve* curves[3] = { nullptr };

    curves[0] = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, false);
    curves[1] = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
    curves[2] = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

    if (curves[0] || curves[1] || curves[2]) {
        FbxTime curTime = startTime;
        FbxTime period;
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
