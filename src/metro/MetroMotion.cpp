#include "MetroMotion.h"
#include "MetroContext.h"

static const size_t kMotionVersionRedux     = 14;  // Last Light has the same version
static const size_t kMotionVersionArktika1  = 17;

enum MotionChunks : size_t {
    MC_HeaderChunk          = 0,
    MC_InfoChunk            = 1,
    MC_DataChunk            = 9,

    // 2033
    MC2033_HeaderChunk      = 0,
    MC2033_InfoChunk        = 1,
    MC2033_BonesMotionChunk = 2
};

enum MotionFlags_2033 : uint8_t {
    MF2033_OffsetsPresent       = 0x01,
    MF2033_RotationsPresent     = 0x02,
    MF2033_NotAnimated          = 0x04,
    MF2033_QuatCompressionEx    = 0x20
};


enum MotionFlags : uint16_t {
    fl_obsolete0 = 0x1,
    fl_looped = 0x2,
    fl_obsolete1 = 0x4,
    fl_exp_ik_xz_rot = 0x8,
    fl_rootmover = 0x10,
    fl_absolute_xform = 0x20,
    fl_delta = 0x40,
    fl_exp_twist = 0x80,
    fl_xform_motion = 0x100,
    fl_exp_root_xz_rot = 0x200,
    fl_disable_ik = 0x400,
    fl_sound = 0x800,
    fl_force_hq = 0x1000,
    rt_streamer_disable = 0x4000,
    rt_streamer_hq = 0x8000,
    rt_streamer_mask = 0xC000
};



template <typename TConvertion, typename TLerpFunc>
static vec4 Util_CurveResolve(const AttributeCurve& curve, const float curveT, TLerpFunc lerpFunc) {
    vec4 result;

    if (curve.points.size() == 1) { // constant value
        result = curve.points.front().value;
    } else {
        const size_t numPoints = curve.points.size();
        size_t pointA = numPoints, pointB = numPoints;
        for (size_t i = 0; i < numPoints; ++i) {
            const auto& p = curve.points[i];
            if (p.time >= curveT) {
                pointB = i;
                break;
            }
        }

        if (pointB == numPoints) {
            pointB--;
            pointA = pointB;
        } else if (pointB == 0) {
            pointA = 0;
        } else {
            pointA = pointB - 1;
        }

        const auto& pA = curve.points[pointA];

        if (pointA == pointB) {
            result = pA.value;
        } else {
            const auto& pB = curve.points[pointB];
            const float t = (curveT - pA.time) / (pB.time - pA.time);

            TConvertion temp = lerpFunc(*rcast<const TConvertion*>(&pA.value), *rcast<const TConvertion*>(&pB.value), t);
            result = *rcast<const vec4*>(&temp);
        }
    }

    return result;
}




size_t MetroMotion::ReadMotionDataHeader(const uint8_t* ptr, MetroMotion::MotionDataHeader& hdr, const size_t version) {
    size_t result = 0;

    hdr.bonesMask.Clear();
    std::memcpy(&hdr.bonesMask.dwords[0], ptr, sizeof(uint32_t[4]));    result += sizeof(uint32_t[4]);
    if (version >= kMotionVersionArktika1) {
        std::memcpy(&hdr.bonesMask.dwords[4], ptr + result, sizeof(uint32_t[4]));   result += sizeof(uint32_t[4]);
    }

    hdr.numLocators = *rcast<const uint16_t*>(ptr + result);    result += sizeof(uint16_t);
    hdr.numXforms = *rcast<const uint16_t*>(ptr + result);      result += sizeof(uint16_t);
    hdr.totalSize = *rcast<const uint32_t*>(ptr + result);      result += sizeof(uint32_t);
    hdr.unknown_0 = *rcast<const uint64_t*>(ptr + result);      result += sizeof(uint64_t);

    if (version < kMotionVersionArktika1) {
        hdr.bonesMask.dwords[0] = EndianSwapBytes(hdr.bonesMask.dwords[0]);
        hdr.bonesMask.dwords[1] = EndianSwapBytes(hdr.bonesMask.dwords[1]);
        hdr.bonesMask.dwords[2] = EndianSwapBytes(hdr.bonesMask.dwords[2]);
        hdr.bonesMask.dwords[3] = EndianSwapBytes(hdr.bonesMask.dwords[3]);

        hdr.numLocators = EndianSwapBytes(hdr.numLocators);
        hdr.numXforms = EndianSwapBytes(hdr.numXforms);
        hdr.totalSize = EndianSwapBytes(hdr.totalSize);
    }

    return result;
}


MetroMotion::MetroMotion(const CharString& name)
    : mName(name)
    // header
    , mVersion(0)
    , mBonesCRC(0)
    , mNumBones(0)
    // info
    , mFlags(0)
    , mSpeed(1.0f)
    , mAccrue(1.0f)
    , mFalloff(1.0f)
    , mNumFrames(0)
    , mJumpFrame(0)
    , mLandFrame(0)
    , mMotionsDataSize(0)
    , mMotionsOffsetsSize(0)
{
    mFramesBitmask.Clear();
    mAffectedBones.Clear();
    mHighQualityBones.Clear();
}
MetroMotion::~MetroMotion() {
}

bool MetroMotion::LoadHeader(MemStream& stream) {
    size_t chunksFound = 0;

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case MC_HeaderChunk: {
                this->ReadHeaderChunk(stream);

                ++chunksFound;
            } break;

            case MC_InfoChunk: {
                this->ReadInfoChunk(stream);

                ++chunksFound;
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    return chunksFound == 2;
}

bool MetroMotion::LoadFromData(MemStream& stream) {
    bool result = false;

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case MC_HeaderChunk: {
                this->ReadHeaderChunk(stream);
            } break;

            case MC_InfoChunk: {
                this->ReadInfoChunk(stream);
            } break;

            case MC_DataChunk: {
                assert(chunkSize == mMotionsDataSize);
                if (chunkSize != mMotionsDataSize) {
                    return false;
                }

                mMotionsData.resize(mMotionsDataSize);
                stream.ReadToBuffer(mMotionsData.data(), mMotionsData.size());
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    result = this->LoadInternal();

    mMotionsData.clear();

    return result;
}

// Metro 2033 reading
bool MetroMotion::LoadHeader_2033(MemStream& stream) {
    size_t chunksFound = 0;

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case MC2033_HeaderChunk: {
                this->ReadHeaderChunk_2033(stream);

                ++chunksFound;
            } break;

            case MC2033_InfoChunk: {
                this->ReadInfoChunk_2033(stream);

                ++chunksFound;
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    return chunksFound == 2;
}

bool MetroMotion::LoadFromData_2033(MemStream& stream) {
    bool result = false;

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case MC2033_HeaderChunk: {
                this->ReadHeaderChunk_2033(stream);

                mMotionDataHeader.bonesMask.Clear();
                std::memcpy(&mMotionDataHeader.bonesMask.dwords[0], &mFramesBitmask.dwords[0], sizeof(mFramesBitmask.dwords));
            } break;

            case MC2033_InfoChunk: {
                this->ReadInfoChunk_2033(stream);
            } break;

            case MC2033_BonesMotionChunk: {
                this->ReadBonesMotionChunk_2033(stream);
                result = true;
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    return result;
}

bool MetroMotion::SaveToStream(MemWriteStream& stream) {
    MemWriteStream motionDataStream;
    this->WriteMotionData(motionDataStream);

    // write header chunk
    {
        ChunkWriteHelper chunkHeader(stream, MC_HeaderChunk);

        stream.WriteU32(scast<uint32_t>(mVersion));
        stream.WriteU32(scast<uint32_t>(mBonesCRC));
        stream.WriteU16(scast<uint16_t>(mNumBones));
        stream.WriteU16(scast<uint16_t>(mNumLocators));
    }
    // write info chunk
    {
        ChunkWriteHelper chunkInfo(stream, MC_InfoChunk);

        stream.WriteU16(scast<uint16_t>(mFlags));

        stream.WriteF32(mSpeed);
        stream.WriteF32(mAccrue);
        stream.WriteF32(mFalloff);

        stream.WriteU32(scast<uint32_t>(mNumFrames));
        stream.WriteU16(scast<uint16_t>(mJumpFrame));
        stream.WriteU16(scast<uint16_t>(mLandFrame));

        stream.Write(&mAffectedBones.dwords[0], sizeof(uint32_t[4]));
        if (mVersion >= kMotionVersionArktika1) {
            stream.Write(&mAffectedBones.dwords[4], sizeof(uint32_t[4]));
        }

        stream.WriteU32(scast<uint32_t>(mMotionsDataSize));
        stream.WriteU32(scast<uint32_t>(mMotionsOffsetsSize));

        stream.Write(&mHighQualityBones.dwords[0], sizeof(uint32_t[4]));
        if (mVersion >= kMotionVersionArktika1) {
            stream.Write(&mHighQualityBones.dwords[4], sizeof(uint32_t[4]));
        }
    }
    // write data
    {
        ChunkWriteHelper chunkHeader(stream, MC_DataChunk);

        stream.Append(motionDataStream);
    }

    return true;
}

const CharString& MetroMotion::GetName() const {
    return mName;
}

size_t MetroMotion::GetBonesCRC() const {
    return mBonesCRC;
}

size_t MetroMotion::GetNumBones() const {
    return mNumBones;
}

size_t MetroMotion::GetNumLocators() const {
    return mNumLocators;
}

size_t MetroMotion::GetNumFrames() const {
    return mNumFrames;
}

float MetroMotion::GetMotionTimeInSeconds() const {
    return scast<float>(mNumFrames) / scast<float>(kFrameRate);
}

bool MetroMotion::IsBoneAnimated(const size_t boneIdx) const {
    const bool bonePresent = mAffectedBones.IsPresent(boneIdx);
    const bool motionHasThisBone = mMotionDataHeader.bonesMask.IsPresent(boneIdx);

    return bonePresent && motionHasThisBone;
}

quat MetroMotion::GetBoneRotation(const size_t boneIdx, const size_t key) const {
    quat result(1.0f, 0.0f, 0.0f, 0.0f);

    if (!mBonesRotations.empty()) {
        const AttributeCurve& curve = mBonesRotations[boneIdx];
        if (!curve.points.empty()) {
            const float timing = scast<float>(key) / scast<float>(kFrameRate);
            vec4 v = Util_CurveResolve<quat>(curve, timing, QuatSlerp);
            result = *rcast<const quat*>(&v);
        }
    }

    return result;
}

vec3 MetroMotion::GetBonePosition(const size_t boneIdx, const size_t key) const {
    vec3 result(0.0f);

    if (!mBonesPositions.empty()) {
        const AttributeCurve& curve = mBonesPositions[boneIdx];
        if (!curve.points.empty()) {
            const float timing = scast<float>(key) / scast<float>(kFrameRate);
            result = Util_CurveResolve<vec4>(curve, timing, Lerp<vec4>);
        }
    }

    return result;
}

vec3 MetroMotion::GetBoneScale(const size_t boneIdx, const size_t key) const {
    vec3 result(0.0f);

    if (!mBonesScales.empty()) {
        const AttributeCurve& curve = mBonesScales[boneIdx];
        if (!curve.points.empty()) {
            const float timing = scast<float>(key) / scast<float>(kFrameRate);
            result = Util_CurveResolve<vec4>(curve, timing, Lerp<vec4>);
        }
    }

    return result;
}

quat MetroMotion::GetLocatorRotation(const size_t boneIdx, const size_t key) const {
    quat result(1.0f, 0.0f, 0.0f, 0.0f);

    if (!mLocatorsRotations.empty()) {
        const AttributeCurve& curve = mLocatorsRotations[boneIdx];
        if (!curve.points.empty()) {
            const float timing = scast<float>(key) / scast<float>(kFrameRate);
            vec4 v = Util_CurveResolve<quat>(curve, timing, QuatSlerp);
            result = *rcast<const quat*>(&v);
        }
    }

    return result;
}

vec3 MetroMotion::GetLocatorPosition(const size_t boneIdx, const size_t key) const {
    vec3 result(0.0f);

    if (!mLocatorsPositions.empty()) {
        const AttributeCurve& curve = mLocatorsPositions[boneIdx];
        if (!curve.points.empty()) {
            const float timing = scast<float>(key) / scast<float>(kFrameRate);
            result = Util_CurveResolve<vec4>(curve, timing, Lerp<vec4>);
        }
    }

    return result;
}

vec3 MetroMotion::GetLocatorScale(const size_t boneIdx, const size_t key) const {
    vec3 result(0.0f);

    if (!mLocatorsScales.empty()) {
        const AttributeCurve& curve = mLocatorsScales[boneIdx];
        if (!curve.points.empty()) {
            const float timing = scast<float>(key) / scast<float>(kFrameRate);
            result = Util_CurveResolve<vec4>(curve, timing, Lerp<vec4>);
        }
    }

    return result;
}



size_t MetroMotion::CalcNumOffsetsToBSwap() const {
    //!! Redux !!
    //#NOTE_SK: now to the weird part - bones tracks contain Q + T + S
    //          so in theory we should swap endianess for all of these offsets
    //          but it seems like there's an error(??) in the data cooking
    //          and only (num_affected_bones * 2) offsets are swapped
    //          oh! and + (numLocators * 3)
    //          oh!! and if numXforms != 0 -> then +2 as well
    const size_t numAffectedBones = mMotionDataHeader.bonesMask.CountOnes();
    const size_t numOffsetsToSwap = (numAffectedBones * 2) + (mMotionDataHeader.numLocators * 3) + (mMotionDataHeader.numXforms ? 2 : 0);
    return numOffsetsToSwap;
}

void MetroMotion::ReadHeaderChunk(MemStream& stream) {
    mVersion = stream.ReadTyped<uint32_t>();
    mBonesCRC = stream.ReadTyped<uint32_t>();
    mNumBones = stream.ReadTyped<uint16_t>();
    mNumLocators = stream.ReadTyped<uint16_t>();
}

void MetroMotion::ReadInfoChunk(MemStream& stream) {
    mFlags = stream.ReadTyped<uint16_t>();

    mSpeed = stream.ReadTyped<float>();
    mAccrue = stream.ReadTyped<float>();
    mFalloff = stream.ReadTyped<float>();

    mNumFrames = stream.ReadTyped<uint32_t>();
    mJumpFrame = stream.ReadTyped<uint16_t>();
    mLandFrame = stream.ReadTyped<uint16_t>();

    stream.ReadToBuffer(&mAffectedBones.dwords[0], sizeof(uint32_t[4]));
    if (mVersion >= kMotionVersionArktika1) {
        stream.ReadToBuffer(&mAffectedBones.dwords[4], sizeof(uint32_t[4]));
    }

    mMotionsDataSize = stream.ReadTyped<uint32_t>();
    mMotionsOffsetsSize = stream.ReadTyped<uint32_t>();

    stream.ReadToBuffer(&mHighQualityBones.dwords[0], sizeof(uint32_t[4]));
    if (mVersion >= kMotionVersionArktika1) {
        stream.ReadToBuffer(&mHighQualityBones.dwords[4], sizeof(uint32_t[4]));
    }
}

// Metro 2033 reading
void MetroMotion::ReadHeaderChunk_2033(MemStream& stream) {
    mVersion = stream.ReadTyped<uint32_t>();
    mBonesCRC = stream.ReadTyped<uint32_t>();
    mNumBones = stream.ReadTyped<uint16_t>();

    if (mVersion > 6) {
        stream.ReadStruct(mFramesBitmask);
    } else {
        mFramesBitmask.Fill();
    }
}

void MetroMotion::ReadInfoChunk_2033(MemStream& stream) {
    mFlags = stream.ReadTyped<uint16_t>();

    if (mVersion > 6) {
        mSpeed = stream.ReadTyped<float>();
        mAccrue = stream.ReadTyped<float>();
        mFalloff = stream.ReadTyped<float>();
    } else if (mVersion < 3) {
        stream.SkipBytes(sizeof(uint16_t)); // ???
    }

    mNumFrames = stream.ReadTyped<uint32_t>();

    if (mVersion > 3) {
        mJumpFrame = stream.ReadTyped<uint16_t>();
        mLandFrame = stream.ReadTyped<uint16_t>();

        if (mVersion > 5) {
            stream.ReadToBuffer(&mAffectedBones.dwords[0], sizeof(uint32_t[4]));
            stream.ReadToBuffer(&mHighQualityBones.dwords[0], sizeof(uint32_t[4]));
        } else {
            mAffectedBones.Fill();
            mHighQualityBones.Fill();
        }
    }
}

void MetroMotion::ReadBonesMotionChunk_2033(MemStream& stream) {
    mBonesRotations.resize(mNumBones);
    mBonesPositions.resize(mNumBones);

    for (size_t boneIdx = 0; boneIdx < mNumBones; ++boneIdx) {
        const bool boneAnimated = mFramesBitmask.IsPresent(boneIdx);
        const bool motionHasThisBone = mAffectedBones.IsPresent(boneIdx);

        if (boneAnimated && motionHasThisBone) {
            this->ReadBoneCurve_2033(stream, mBonesRotations[boneIdx], mBonesPositions[boneIdx]);
        }
    }
}

enum class AttribCurveType : uint8_t {
    Invalid         = 0,
    Uncompressed    = 1,    // raw float values
    OneValue        = 2,    // constant value, no curve
    Unknown_3       = 3,
    CompressedPos   = 4,    // quantized position, scale + offset + u16 values
    CompressedQuat  = 5,    // quantized quaternion (xyz, we restore w), s16_snorm values
    Unknown_6       = 6,
    Empty           = 7     // no curve, why not just filter it out with mask ???
};

bool MetroMotion::LoadInternal() {
    bool result = false;

    if (!mMotionsData.empty() && mMotionsData.size() > mMotionsOffsetsSize) {
        uint8_t* ptr = mMotionsData.data();

        const size_t offsetsTableOffset = ReadMotionDataHeader(ptr, mMotionDataHeader, mVersion);
        uint32_t* offsetsTable = rcast<uint32_t*>(ptr + offsetsTableOffset);

        size_t stopBswapIdx = 0;
        if (mVersion < kMotionVersionArktika1) {
            //!! Redux !!
            const size_t numOffsetsToSwap = this->CalcNumOffsetsToBSwap();
            for (size_t i = 0; i < numOffsetsToSwap; ++i) {
                offsetsTable[i] = EndianSwapBytes(offsetsTable[i]);
            }
            stopBswapIdx = numOffsetsToSwap;
        }

        mBonesRotations.resize(mNumBones);
        mBonesPositions.resize(mNumBones);

        const size_t offsetsStride = (MetroContext::Get().GetGameVersion() == MetroGameVersion::OGLastLight) ? 2 : 3;
        if (offsetsStride == 3) {
            mBonesScales.resize(mNumBones);
        }

        size_t flatIdx = 0;
        for (size_t boneIdx = 0; boneIdx < mNumBones; ++boneIdx) {
            const bool bonePresent = mAffectedBones.IsPresent(boneIdx);
            const bool motionHasThisBone = mMotionDataHeader.bonesMask.IsPresent(boneIdx);

            if (bonePresent && motionHasThisBone) {
                const size_t offsetQIdx = flatIdx * offsetsStride + 0;
                const size_t offsetTIdx = flatIdx * offsetsStride + 1;

                const size_t offsetQ = offsetsTable[offsetQIdx];
                const size_t offsetT = offsetsTable[offsetTIdx];

                this->ReadAttributeCurve(ptr + offsetQ, mBonesRotations[boneIdx], 4, offsetQIdx >= stopBswapIdx);
                this->ReadAttributeCurve(ptr + offsetT, mBonesPositions[boneIdx], 3, offsetTIdx >= stopBswapIdx);

                if (offsetsStride == 3) {
                    const size_t offsetSIdx = flatIdx * offsetsStride + 2;
                    const size_t offsetS = offsetsTable[offsetSIdx];
                    this->ReadAttributeCurve(ptr + offsetS, mBonesScales[boneIdx], 3, offsetSIdx >= stopBswapIdx);
                }

                ++flatIdx;
            }
        }

        if (mNumLocators) {
            mLocatorsRotations.resize(mNumLocators);
            mLocatorsPositions.resize(mNumLocators);
            if (offsetsStride == 3) {
                mLocatorsScales.resize(mNumLocators);
            }

            for (size_t locatorIdx = 0; locatorIdx < mNumLocators; ++locatorIdx, ++flatIdx) {
                const size_t offsetQIdx = flatIdx * offsetsStride + 0;
                const size_t offsetTIdx = flatIdx * offsetsStride + 1;

                const size_t offsetQ = offsetsTable[offsetQIdx];
                const size_t offsetT = offsetsTable[offsetTIdx];

                this->ReadAttributeCurve(ptr + offsetQ, mLocatorsRotations[locatorIdx], 4, offsetQIdx >= stopBswapIdx);
                this->ReadAttributeCurve(ptr + offsetT, mLocatorsPositions[locatorIdx], 3, offsetTIdx >= stopBswapIdx);

                if (offsetsStride == 3) {
                    const size_t offsetSIdx = flatIdx * offsetsStride + 2;
                    const size_t offsetS = offsetsTable[offsetSIdx];
                    this->ReadAttributeCurve(ptr + offsetS, mLocatorsScales[locatorIdx], 3, offsetTIdx >= stopBswapIdx);
                }

                ++flatIdx;
            }
        }

#if 0
        if (mMotionDataHeader.numXforms) {
            mXFormsRotations.resize(mMotionDataHeader.numXforms);
            mXFormsPositions.resize(mMotionDataHeader.numXforms);
            mXFormsScales.resize(mMotionDataHeader.numXforms);

            for (size_t xformIdx = 0; xformIdx < mMotionDataHeader.numXforms; ++xformIdx) {
                const size_t offsetQIdx = flatIdx * offsetsStride + 0;
                const size_t offsetTIdx = flatIdx * offsetsStride + 1;

                const size_t offsetQ = offsetsTable[offsetQIdx];
                const size_t offsetT = offsetsTable[offsetTIdx];

                this->ReadAttributeCurve(ptr + offsetQ, mXFormsRotations[xformIdx], 4, offsetQIdx >= stopBswapIdx);
                this->ReadAttributeCurve(ptr + offsetT, mXFormsPositions[xformIdx], 3, offsetTIdx >= stopBswapIdx);

                if (offsetsStride == 3) {
                    const size_t offsetSIdx = flatIdx * offsetsStride + 2;
                    const size_t offsetS = offsetsTable[offsetSIdx];
                    this->ReadAttributeCurve(ptr + offsetS, mXFormsScales[xformIdx], 3, offsetTIdx >= stopBswapIdx);
                }

                ++flatIdx;
            }
        }
#endif

        result = true;
    }

    return result;
}

inline vec3 EndianSwapBytes(const vec3& v) {
    return vec3(EndianSwapBytes(v.x), EndianSwapBytes(v.y), EndianSwapBytes(v.z));
}

inline vec4 EndianSwapBytes(const vec4& v) {
    return vec4(EndianSwapBytes(v.x), EndianSwapBytes(v.y), EndianSwapBytes(v.z), EndianSwapBytes(v.w));
}

void MetroMotion::ReadAttributeCurve(const uint8_t* curveData, AttributeCurve& curve, const size_t attribSize, const bool disableBswap) {
    uint32_t curveHeader = *rcast<const uint32_t*>(curveData);
    if (mVersion < kMotionVersionArktika1 && !disableBswap) {
        curveHeader = EndianSwapBytes(curveHeader);
    }

    const size_t numPoints = scast<size_t>(curveHeader & 0xFFFF);
    const size_t pointSize = ((curveHeader >> 24) & 0xF);
    const AttribCurveType ctype = scast<AttribCurveType>((curveHeader >> 16) & 0xF);

    assert(pointSize == attribSize);
    assert(attribSize <= 4);

    curveData += 4;

    if (ctype == AttribCurveType::Empty) {
        curve.points.clear();
    } else if (ctype == AttribCurveType::OneValue) {
        curve.points.resize(1);
        auto& p = curve.points.back();
        p.time = 0.0f;
        memcpy(&p.value, curveData, attribSize * sizeof(float));

        if (mVersion < kMotionVersionArktika1 && !disableBswap) {
            p.value = EndianSwapBytes(p.value);
        }
    } else if (ctype == AttribCurveType::Unknown_3 || ctype == AttribCurveType::Unknown_6) {
        assert(false);
    } else {
        curve.points.resize(numPoints);

        switch (ctype) {
            case AttribCurveType::Uncompressed: {
                const float* timingsPtr = rcast<const float*>(curveData);
                const float* valuesPtr = rcast<const float*>(curveData + (numPoints * sizeof(float)));

                for (auto& p : curve.points) {
                    p.time = *timingsPtr;
                    memcpy(&p.value, valuesPtr, attribSize * sizeof(float));

                    if (mVersion < kMotionVersionArktika1 && !disableBswap) {
                        p.time = EndianSwapBytes(p.time);
                        p.value = EndianSwapBytes(p.value);
                    }

                    timingsPtr++;
                    valuesPtr += attribSize;
                }
            } break;

            case AttribCurveType::CompressedPos: {
                float timingScale = *rcast<const float*>(curveData);
                curveData += 4;

                vec3 scale = rcast<const vec3*>(curveData)[0];
                vec3 offset = rcast<const vec3*>(curveData)[1];
                curveData += sizeof(vec3[2]);

                if (mVersion < kMotionVersionArktika1 && !disableBswap) {
                    timingScale = 1.0f / EndianSwapBytes(timingScale);
                    scale = EndianSwapBytes(scale);
                    offset = EndianSwapBytes(offset);
                } else {
                    timingScale = 1.0f / timingScale;
                }

                const uint16_t* timingsPtr = rcast<const uint16_t*>(curveData);
                const uint16_t* valuesPtr = rcast<const uint16_t*>(curveData + (numPoints * sizeof(uint16_t)));

                for (auto& p : curve.points) {
                    uint16_t pt = *timingsPtr;
                    uint16_t px = valuesPtr[0];
                    uint16_t py = valuesPtr[1];
                    uint16_t pz = valuesPtr[2];

                    if (mVersion < kMotionVersionArktika1 && !disableBswap) {
                        pt = EndianSwapBytes(pt);
                        px = EndianSwapBytes(px);
                        py = EndianSwapBytes(py);
                        pz = EndianSwapBytes(pz);
                    }

                    p.time = scast<float>(pt) * timingScale;

                    p.value.x = scast<float>(px) * scale.x + offset.x;
                    p.value.y = scast<float>(py) * scale.y + offset.y;
                    p.value.z = scast<float>(pz) * scale.z + offset.z;

                    timingsPtr++;
                    valuesPtr += 3;
                }
            } break;

            case AttribCurveType::CompressedQuat: {
                const float normFactor = 0.0000215805f; // (1 / 32766) * (1 / sqrt(2))
                float timingScale = *rcast<const float*>(curveData);
                curveData += 4;

                if (mVersion < kMotionVersionArktika1 && !disableBswap) {
                    timingScale = 1.0f / EndianSwapBytes(timingScale);
                } else {
                    timingScale = 1.0f / timingScale;
                }

                const uint16_t* timingsPtr = rcast<const uint16_t*>(curveData);
                const int16_t* valuesPtr = rcast<const int16_t*>(curveData + (numPoints * sizeof(uint16_t)));

                for (auto& p : curve.points) {
                    uint16_t pt = *timingsPtr;
                    int16_t px = valuesPtr[0];
                    int16_t py = valuesPtr[1];
                    int16_t pz = valuesPtr[2];

                    if (mVersion < kMotionVersionArktika1 && !disableBswap) {
                        pt = EndianSwapBytes(pt);
                        px = EndianSwapBytes(px);
                        py = EndianSwapBytes(py);
                        pz = EndianSwapBytes(pz);
                    }

                    p.time = scast<float>(pt) * timingScale;

                    const int permutation = (py & 1) | (2 * (px & 1));
                    const int wsign = (pz & 1);

                    const float qx = scast<float>(px) * normFactor;
                    const float qy = scast<float>(py) * normFactor;
                    const float qz = scast<float>(pz) * normFactor;
                    const float t = 1.0f - (qx * qx) - (qy * qy) - (qz * qz);
                    const float qw = (t < 0.0f) ? 0.0f : (wsign ? -std::sqrtf(t) : std::sqrtf(t));

                    switch (permutation) {
                        case 0: p.value = vec4(qw, qx, qy, qz); break;
                        case 1: p.value = vec4(qx, qw, qy, qz); break;
                        case 2: p.value = vec4(qx, qy, qw, qz); break;
                        case 3: p.value = vec4(qx, qy, qz, qw); break;
                    }

                    *rcast<quat*>(&p.value) = Normalize(*rcast<quat*>(&p.value));

                    timingsPtr++;
                    valuesPtr += 3;
                }
            } break;
        }
    }
}

void MetroMotion::ReadBoneCurve_2033(MemStream& stream, AttributeCurve& rotations, AttributeCurve& positions) {
    const float secPerFrame = 1.0f / scast<float>(kFrameRate);
    const float normFactor = 0.0000215805f;

    const uint8_t flags = stream.ReadTyped<uint8_t>();
    const uint32_t curveCrc = stream.ReadTyped<uint32_t>();
    const uint32_t unknown = stream.ReadTyped<uint32_t>();

    const size_t numRotations = ((flags & MF2033_RotationsPresent) != 0) ? mNumFrames : 1;

    rotations.points.resize(numRotations);
    float time = 0.0f;
    for (auto& pt : rotations.points) {
        pt.time = time;

        const int16_t i0 = stream.ReadTyped<int16_t>();
        const int16_t i1 = stream.ReadTyped<int16_t>();
        const int16_t i2 = stream.ReadTyped<int16_t>();

        if (flags & MF2033_QuatCompressionEx) {
            const int permutation = (i1 & 1) | (2 * (i0 & 1));
            //const int wsign = (i2 & 1);

            const float qx = static_cast<float>(i0) * normFactor;
            const float qy = static_cast<float>(i1) * normFactor;
            const float qz = static_cast<float>(i2) * normFactor;
            const float t = 1.0f - (qx * qx) - (qy * qy) - (qz * qz);
            //const float qw = (t < 0.0f) ? 0.0f : (wsign ? -std::sqrtf(t) : std::sqrtf(t));
            const float qw = (t < 0.0f) ? 0.0f : sqrt(t);

            switch (permutation) {
                case 0: pt.value = vec4(qw, qx, qy, qz); break;
                case 1: pt.value = vec4(qx, qw, qy, qz); break;
                case 2: pt.value = vec4(qx, qy, qw, qz); break;
                case 3: pt.value = vec4(qx, qy, qz, qw); break;
            }
        } else {
            const float qx = static_cast<float>(i0) / 32767.0f;
            const float qy = static_cast<float>(i1) / 32767.0f;
            const float qz = static_cast<float>(i2) / 32767.0f;
            const float t = 1.0f - (qx * qx) - (qy * qy) - (qz * qz);
            const float qw = (t < 0.0f) ? 0.0f : sqrt(t);

            pt.value = vec4(qx, qy, qz, qw);
        }

        *rcast<quat*>(&pt.value) = Normalize(*rcast<quat*>(&pt.value));

        time += secPerFrame;
    }

    if (flags & MF2033_OffsetsPresent) {
        stream.SkipBytes(sizeof(uint32_t[2])); // WTF ???

        MyArray<int16_t> elements(mNumFrames * 3);
        stream.ReadToBuffer(elements.data(), mNumFrames * sizeof(int16_t[3]));

        vec3 scale, offset;
        stream.ReadStruct(scale);
        stream.ReadStruct(offset);

        const int16_t* elementsPtr = elements.data();

        positions.points.resize(mNumFrames);
        time = 0.0f;
        for (auto& pt : positions.points) {
            pt.time = time;

            pt.value = vec4(vec3(scast<float>(elementsPtr[0]), scast<float>(elementsPtr[1]), scast<float>(elementsPtr[2])) * scale + offset, 0.0f);

            time += secPerFrame;
            elementsPtr += 3;
        }
    } else {
        positions.points.resize(1);
        auto& pt = positions.points.front();
        pt.time = 0.0f;
        stream.ReadToBuffer(&pt.value, sizeof(vec3));
    }
}

void MetroMotion::WriteMotionData(MemWriteStream& stream) {
    MemWriteStream motionDataHdrStream;

    size_t totalSizeOffset = 0;
    if (mVersion >= kMotionVersionArktika1) {
        motionDataHdrStream.Write(&mMotionDataHeader.bonesMask, sizeof(mMotionDataHeader.bonesMask));

        motionDataHdrStream.WriteU16(mMotionDataHeader.numLocators);
        motionDataHdrStream.WriteU16(mMotionDataHeader.numXforms);
        totalSizeOffset = motionDataHdrStream.GetWrittenBytesCount();
        motionDataHdrStream.WriteU32(mMotionDataHeader.totalSize);
        motionDataHdrStream.WriteU64(mMotionDataHeader.unknown_0);
    } else {
        for (size_t i = 0; i < 4; ++i) {
            motionDataHdrStream.WriteU32(EndianSwapBytes(mMotionDataHeader.bonesMask.dwords[i]));
        }

        motionDataHdrStream.WriteU16(EndianSwapBytes(mMotionDataHeader.numLocators));
        motionDataHdrStream.WriteU16(EndianSwapBytes(mMotionDataHeader.numXforms));
        totalSizeOffset = motionDataHdrStream.GetWrittenBytesCount();
        motionDataHdrStream.WriteU32(EndianSwapBytes(mMotionDataHeader.totalSize));
        motionDataHdrStream.WriteU64(EndianSwapBytes(mMotionDataHeader.unknown_0));
    }

    MemWriteStream curvesDataStream;
    const size_t numOffsetsToSwap = (mVersion < kMotionVersionArktika1) ? this->CalcNumOffsetsToBSwap() : 0;

    MyArray<uint32_t> offsetsTable;
    size_t flatIdx = 0;
    for (size_t i = 0; i < mNumBones; ++i) {
        // Q
        offsetsTable.push_back(scast<uint32_t>(curvesDataStream.GetWrittenBytesCount()));
        this->WriteMotionCurve(curvesDataStream, mBonesRotations[i], 4, flatIdx >= numOffsetsToSwap);
        ++flatIdx;
        // T
        offsetsTable.push_back(scast<uint32_t>(curvesDataStream.GetWrittenBytesCount()));
        this->WriteMotionCurve(curvesDataStream, mBonesPositions[i], 3, flatIdx >= numOffsetsToSwap);
        ++flatIdx;
        // S
        offsetsTable.push_back(scast<uint32_t>(curvesDataStream.GetWrittenBytesCount()));
        this->WriteMotionCurve(curvesDataStream, mBonesScales[i], 3, flatIdx >= numOffsetsToSwap);
        ++flatIdx;
    }

    // fix-up our offsets
    const uint32_t offsetsTableSize = scast<uint32_t>(offsetsTable.size() * sizeof(uint32_t));
    const uint32_t offsetAddition = scast<uint32_t>(motionDataHdrStream.GetWrittenBytesCount() + offsetsTableSize);
    flatIdx = 0;
    for (uint32_t& offset : offsetsTable) {
        offset += offsetAddition;

        if (flatIdx < numOffsetsToSwap) {
            offset = EndianSwapBytes(offset);
        }

        ++flatIdx;
    }

    mMotionsDataSize = offsetAddition + curvesDataStream.GetWrittenBytesCount();
    mMotionsOffsetsSize = offsetsTableSize;

    uint32_t& totalSizeToFixUp = *rcast<uint32_t*>(rcast<uint8_t*>(motionDataHdrStream.Data()) + totalSizeOffset);
    totalSizeToFixUp = (mVersion < kMotionVersionArktika1) ? EndianSwapBytes(scast<uint32_t>(mMotionsDataSize)) : scast<uint32_t>(mMotionsDataSize);

    // header
    stream.Append(motionDataHdrStream);
    // offsets table
    stream.Write(offsetsTable.data(), offsetsTableSize);
    // curves data
    stream.Append(curvesDataStream);
}

void MetroMotion::WriteMotionCurve(MemWriteStream& stream, const AttributeCurve& curve, const size_t attribSize, const bool disableBswap) {
    const uint32_t numPoints = scast<uint32_t>(curve.points.size()) & 0xFFFF;
    const uint32_t pointSize = attribSize & 0xF;
    uint32_t ctype = scast<uint32_t>(AttribCurveType::Empty);

    if (curve.points.size() == 1) {
        ctype = scast<uint32_t>(AttribCurveType::OneValue);
    } else if (curve.points.size() > 1) {
        ctype = scast<uint32_t>(AttribCurveType::Uncompressed);
    }

    const uint32_t curveHeader = (pointSize << 24) | (ctype << 16) | numPoints;
    stream.WriteU32(disableBswap ? curveHeader : EndianSwapBytes(curveHeader));

    if (ctype == scast<uint32_t>(AttribCurveType::OneValue)) {
        vec4 value = curve.points.back().value;
        for (size_t i = 0; i < attribSize; ++i) {
            stream.WriteF32(disableBswap ? value[i] : EndianSwapBytes(value[i]));
        }
    } else if (ctype == scast<uint32_t>(AttribCurveType::Uncompressed)) {
        // timings
        for (const auto& p : curve.points) {
            stream.WriteF32(disableBswap ? p.time : EndianSwapBytes(p.time));
        }
        // values
        for (const auto& p : curve.points) {
            vec4 value = p.value;
            for (size_t i = 0; i < attribSize; ++i) {
                stream.WriteF32(disableBswap ? value[i] : EndianSwapBytes(value[i]));
            }
        }
    }
}
