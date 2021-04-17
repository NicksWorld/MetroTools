#include "ImporterFBX.h"

#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"

#define FBXSDK_NEW_API
#define FBXSDK_SHARED
#include "fbxsdk.h"

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


vec3 FbxVecToVec3(const FbxVector4& v) {
    return vec3(scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
vec3 FbxDbl3ToVec3(const FbxDouble3& v) {
    return vec3(scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}
quat FbxQuatToQuat(const FbxQuaternion& v) {
    return quat(scast<float>(v[3]), scast<float>(v[0]), scast<float>(v[1]), scast<float>(v[2]));
}


static FbxVector4 MetroVecToFbxVec(const vec3& v) {
    return FbxVector4(v.x, v.y, v.z);
}

static FbxVector4 MetroRotToFbxRot(const quat& q) {
    vec3 euler = QuatToEuler(q);
    return FbxVector4(Rad2Deg(euler.x), Rad2Deg(euler.y), Rad2Deg(euler.z));
}


ImporterFBX::ImporterFBX() {

}
ImporterFBX::~ImporterFBX() {

}


void ImporterFBX::SetSkeleton(const fs::path& path) {
    MemStream stream = ReadOSFile(path);
    if (stream) {
        mSkeleton.reset(new MetroSkeleton());
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
        ios->SetBoolProp(IMP_META_DATA, true); //we want the meta data so we can get run-time expressions stored in the notes

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
