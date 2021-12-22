#pragma once
#include "MetroTypes.h"

struct AttributeCurve {
    struct AttribPoint {
        float   time;
        vec4    value;
    };

    MyArray<AttribPoint> points;
};

class MetroMotion {
    PACKED_STRUCT_BEGIN
    struct MotionDataHeader {       // size = 48 bytes
        Bitset256   bonesMask;
        uint16_t    numLocators;
        uint16_t    numXforms;
        uint32_t    totalSize;
        float       twoFloats[2];   // always 1.0f, 1.0f ???
    } PACKED_STRUCT_END;
    static_assert(sizeof(MotionDataHeader) == 48);

    static size_t ReadMotionDataHeader(const uint8_t* ptr, MotionDataHeader& hdr, const size_t version);

public:
    static const size_t kFrameRate = 30;

public:
    MetroMotion(const CharString& name = kEmptyString);
    ~MetroMotion();

    bool                    LoadHeader(MemStream& stream);
    bool                    LoadFromData(MemStream& stream);

    // Metro 2033 reading
    bool                    LoadHeader_2033(MemStream& stream);
    bool                    LoadFromData_2033(MemStream& stream);

    bool                    SaveToStream(MemWriteStream& stream);

    const CharString&       GetName() const;

    size_t                  GetBonesCRC() const;
    size_t                  GetNumBones() const;
    size_t                  GetNumLocators() const;
    size_t                  GetNumFrames() const;
    float                   GetMotionTimeInSeconds() const;

    // bones
    bool                    IsBoneAnimated(const size_t boneIdx) const;
    quat                    GetBoneRotation(const size_t boneIdx, const size_t key) const;
    vec3                    GetBonePosition(const size_t boneIdx, const size_t key) const;
    vec3                    GetBoneScale(const size_t boneIdx, const size_t key) const;

    // locators
    quat                    GetLocatorRotation(const size_t boneIdx, const size_t key) const;
    vec3                    GetLocatorPosition(const size_t boneIdx, const size_t key) const;
    vec3                    GetLocatorScale(const size_t boneIdx, const size_t key) const;

//private:
    size_t                  CalcNumOffsetsToBSwap() const;
    void                    ReadHeaderChunk(MemStream& stream);
    void                    ReadInfoChunk(MemStream& stream);
    // Metro 2033 reading
    void                    ReadHeaderChunk_2033(MemStream& stream);
    void                    ReadInfoChunk_2033(MemStream& stream);
    void                    ReadBonesMotionChunk_2033(MemStream& stream);

    bool                    LoadInternal();
    void                    ReadAttributeCurve(const uint8_t* curveData, AttributeCurve& curve, const size_t attribSize, const bool disableBswap);
    void                    ReadBoneCurve_2033(MemStream& stream, AttributeCurve& rotations, AttributeCurve& positions);

    void                    WriteMotionData(MemWriteStream& stream);
    void                    WriteMotionCurve(MemWriteStream& stream, const AttributeCurve& curve, const size_t attribSize, const bool disableBswap);

//private:
    CharString              mName;

    // header
    size_t                  mVersion;
    size_t                  mBonesCRC;
    size_t                  mNumBones;
    size_t                  mNumLocators;
    Bitset128               mFramesBitmask; // 2033 only!
    // info
    size_t                  mFlags;
    float                   mSpeed;
    float                   mAccrue;
    float                   mFalloff;
    size_t                  mNumFrames;
    size_t                  mJumpFrame;
    size_t                  mLandFrame;
    Bitset256               mAffectedBones;
    size_t                  mMotionsDataSize;
    size_t                  mMotionsOffsetsSize;
    Bitset256               mHighQualityBones;
    // data
    MotionDataHeader        mMotionDataHeader;  // Last Light and newer
    BytesArray              mMotionsData;
    // curves
    //  bones
    MyArray<AttributeCurve> mBonesRotations;
    MyArray<AttributeCurve> mBonesPositions;
    MyArray<AttributeCurve> mBonesScales;
    //  locators
    MyArray<AttributeCurve> mLocatorsRotations;
    MyArray<AttributeCurve> mLocatorsPositions;
    MyArray<AttributeCurve> mLocatorsScales;
    //  xforms
    MyArray<AttributeCurve> mXFormsRotations;
    MyArray<AttributeCurve> mXFormsPositions;
    MyArray<AttributeCurve> mXFormsScales;
};
