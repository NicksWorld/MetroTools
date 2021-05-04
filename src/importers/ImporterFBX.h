#pragma once
#include "mycommon.h"
#include "metro/MetroTypes.h"

class MetroMotion;
class MetroSkeleton;

namespace fbxsdk {
    class FbxNode;
    class FbxAnimLayer;
    class FbxTime;
} // namespace fbxsdk



class ImporterFBX {
    struct BoneFbxNode {
        fbxsdk::FbxNode* fbxNode;
        CharString       name;
        size_t           skeletonAttpIdx;
        size_t           skeletonAttpParentIdx;
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

    void    SetSkeleton(const fs::path& path);
    bool    ImportAnimation(const fs::path& path, MetroMotion& motion);

private:
    void    CollectFbxBones(fbxsdk::FbxNode* node);
    void    GatherRotCurve(fbxsdk::FbxAnimLayer* animLayer, fbxsdk::FbxNode* node, AnimCurve::RotCurve& rotCurve, const fbxsdk::FbxTime& startTime, const size_t numFrames);
    void    GatherPosCurve(fbxsdk::FbxAnimLayer* animLayer, fbxsdk::FbxNode* node, AnimCurve::PosCurve& posCurve, const fbxsdk::FbxTime& startTime, const size_t numFrames);

private:
    StrongPtr<MetroSkeleton>    mSkeleton;
    MyArray<BoneFbxNode>        mFbxBones;
    MyArray<BoneFbxNode>        mFbxLocators;
    MyArray<AnimCurve>          mBonesCurves;
    MyArray<AnimCurve>          mLocatorsCurves;
    double                      mAnimStartTime;
    double                      mAnimEndTime;
};
