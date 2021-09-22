#pragma once
#include "mycommon.h"
#include "metro/MetroTypes.h"

class MetroMotion;
class MetroSkeleton;
class MetroModelBase;

namespace fbxsdk {
    class FbxNode;
    class FbxMesh;
    class FbxAnimLayer;
    class FbxTime;
} // namespace fbxsdk



class ImporterFBX {
public:
    struct UniversalVertex {
        struct BoneInfluence {
            uint32_t idx;
            float    weight;
        };

        vec3 pos;
        vec3 normal;
        vec3 tangent;
        vec3 bitangent;
        vec2 uv;
        MyArray<BoneInfluence> boneInfluences;

        bool operator ==(const UniversalVertex& other) const {
            if (this->pos != other.pos) {
                return false;
            }
            if (this->normal != other.normal) {
                return false;
            }
            if (this->tangent != other.tangent) {
                return false;
            }
            if (this->bitangent != other.bitangent) {
                return false;
            }
            if (this->uv != other.uv) {
                return false;
            }

            // ignore boneInfluences for now

            return true;
        }
    };

    struct BoneFbxNode {
        fbxsdk::FbxNode* fbxNode;
        CharString       name;
        size_t           skeletonAttpIdx;
        size_t           skeletonAttpParentIdx;
    };

    struct JointFromFbx {
        fbxsdk::FbxNode* fbxNode;
        fbxsdk::FbxNode* fbxParentNode;
    };

    template <typename T>
    struct AttribCurve {
        struct CurvePoint {
            float   time;
            T       value;
        };

        MyArray<CurvePoint> points;
    };

    struct AnimCurve {
        using RotCurve = AttribCurve<quat>;
        using PosCurve = AttribCurve<vec3>;
        using ScaleCurve = AttribCurve<vec3>;

        RotCurve    rotCurve;
        PosCurve    posCurve;
        ScaleCurve  scaleCurve;
    };

public:
    ImporterFBX();
    ~ImporterFBX();

    void                        SetGameVersion(const MetroGameVersion gameVersion);
    void                        SetSkeleton(const fs::path& path);
    bool                        ImportAnimation(const fs::path& path, MetroMotion& motion);

    RefPtr<MetroModelBase>      ImportModel(const fs::path& filePath);

private:
    void                        AddMeshToModel(RefPtr<MetroModelBase>& model, fbxsdk::FbxMesh* fbxMesh);
    RefPtr<MetroSkeleton>       TryToImportSkeleton(fbxsdk::FbxNode* fbxRootNode);
    void                        AddJointRecursive(fbxsdk::FbxNode* fbxNode, fbxsdk::FbxNode* fbxParentNode);
    void                        FixUpSkinnedVertices(MyArray<UniversalVertex>& vertices);
    BytesArray                  BuildBonesRemapTable(MyArray<UniversalVertex>& vertices);
    void                        CollectFbxBones(fbxsdk::FbxNode* node);
    void                        GatherRotCurve(fbxsdk::FbxAnimLayer* animLayer, fbxsdk::FbxNode* node, AnimCurve::RotCurve& rotCurve, const fbxsdk::FbxTime& startTime, const size_t numFrames);
    void                        GatherPosCurve(fbxsdk::FbxAnimLayer* animLayer, fbxsdk::FbxNode* node, AnimCurve::PosCurve& posCurve, const fbxsdk::FbxTime& startTime, const size_t numFrames);

private:
    MetroGameVersion            mGameVersion;
    MyArray<JointFromFbx>       mJointsBones;
    MyArray<JointFromFbx>       mJointsLocators;
    RefPtr<MetroSkeleton>       mSkeleton;
    MyArray<BoneFbxNode>        mFbxBones;
    MyArray<BoneFbxNode>        mFbxLocators;
    MyArray<AnimCurve>          mBonesCurves;
    MyArray<AnimCurve>          mLocatorsCurves;
    double                      mAnimStartTime;
    double                      mAnimEndTime;
};
