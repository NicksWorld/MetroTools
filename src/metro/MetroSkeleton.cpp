#include "MetroSkeleton.h"
#include "MetroBinArchive.h"
#include "MetroContext.h"
#include "MetroMotion.h"

#include "reflection/MetroReflection.h"

static const uint32_t kSkeletonVersion2033          = 1;
static const uint32_t kSkeletonVersionLastLight     = 5;    // Latest Steam LL version
static const uint32_t kSkeletonVersionRedux         = 8;    // Latest Steam Redux are this version

constexpr size_t PackSkeletonVersions(const uint32_t skelVersion, const uint32_t proceduralVersion) {
    return (scast<size_t>(proceduralVersion) << 32) | skelVersion;
}
constexpr uint32_t UnpackSkeletonVersion(const size_t packedVersions) {
    return scast<uint32_t>(packedVersions & 0xFFFFFFFF);
}
constexpr uint32_t UnpackProceduralVersion(const size_t packedVersions) {
    return scast<uint32_t>((packedVersions >> 32) & 0xFFFFFFFF);
}


enum SkeletonChunks2033 : size_t {
    SC2033_Version      = 1,
    SC2033_Bones        = 13,   // MetroBone
    SC2033_Locators     = 14,   // MetroLocator
    SC2033_Partitions   = 17,   // MetroPartition
    SC2033_Motions      = 19,   // StringZ
    SC2033_IKLocks      = 23,   // MetroIkLock
    SC2033_FaceFX       = 26,   // StringZ
    SC2033_Params       = 27,   // MetroSkelParam
    SC2033_IKChains     = 28,   // MetroIkChain
};

struct ReduxBoneBodyPartHelper {
    uint16_t bp;

    void Serialize(MetroReflectionStream& stream) {
        METRO_SERIALIZE_MEMBER(stream, bp);
    }
};


void ParentMapped::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, parent_bone);
    METRO_SERIALIZE_MEMBER(stream, self_bone);
    METRO_SERIALIZE_MEMBER(stream, q);
    METRO_SERIALIZE_MEMBER(stream, t);
    METRO_SERIALIZE_MEMBER(stream, s);
}

void MetroBoneBase::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, name);
    METRO_SERIALIZE_MEMBER(stream, parent);
    if (stream.IsOut()) {
        this->q = QuatConjugate(q);
    }
    METRO_SERIALIZE_MEMBER(stream, q);
    METRO_SERIALIZE_MEMBER(stream, t);

    this->q = QuatConjugate(q);
}

void MetroBone::Serialize(MetroReflectionStream& stream) {
    MetroBoneBase::Serialize(stream);

    const uint32_t skeletonVersion = UnpackSkeletonVersion(stream.GetUserData());
    if (skeletonVersion >= 19) {
        METRO_SERIALIZE_MEMBER(stream, bp);
        METRO_SERIALIZE_MEMBER(stream, bpf);
    } else {
        //#NOTE_SK: using a hack to serialize old (Redux) bones
        ReduxBoneBodyPartHelper helper{ this->bp };
        stream >> helper;

        this->bp = scast<uint8_t>(helper.bp & 0xFF);
    }
}

void MetroLocator::Serialize(MetroReflectionStream& stream) {
    MetroBoneBase::Serialize(stream);

    METRO_SERIALIZE_MEMBER(stream, fl);
}

void MetroAuxBone::Serialize(MetroReflectionStream& stream) {
    MetroBoneBase::Serialize(stream);

    const uint32_t skeletonVersion = UnpackSkeletonVersion(stream.GetUserData());
    if (skeletonVersion >= 16) {
        METRO_SERIALIZE_MEMBER(stream, fl);
    }
}

// PROCEDURAL BONES start

void MetroProceduralBone::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, type);
    METRO_SERIALIZE_MEMBER(stream, index_in_array);
}


void MetroDrivenBone::Serialize(MetroReflectionStream& stream) {
    const uint32_t proceduralVersion = UnpackProceduralVersion(stream.GetUserData());

    METRO_SERIALIZE_MEMBER_CHOOSE(stream, bone);           // choose
    METRO_SERIALIZE_MEMBER_CHOOSE(stream, driver);         // choose
    METRO_SERIALIZE_MEMBER_CHOOSE(stream, driver_parent);  // choose
    METRO_SERIALIZE_MEMBER(stream, component);
    METRO_SERIALIZE_MEMBER(stream, twister);
    METRO_SERIALIZE_MEMBER(stream, value_min);
    METRO_SERIALIZE_MEMBER(stream, value_max);

    if (proceduralVersion >= 1) {
        METRO_SERIALIZE_MEMBER(stream, refresh_kids);
    }
    if (proceduralVersion >= 5) {
        METRO_SERIALIZE_MEMBER(stream, use_anim_poses);
    }
}

void MetroDynamicBone::Serialize(MetroReflectionStream& stream) {
    const uint32_t proceduralVersion = UnpackProceduralVersion(stream.GetUserData());

    METRO_SERIALIZE_MEMBER_CHOOSE(stream, bone);           // choose
    METRO_SERIALIZE_MEMBER(stream, inertia);
    METRO_SERIALIZE_MEMBER(stream, damping);

    if (proceduralVersion >= 9) {
        METRO_SERIALIZE_MEMBER(stream, pos_min_limits);
        METRO_SERIALIZE_MEMBER(stream, pos_max_limits);
        METRO_SERIALIZE_MEMBER(stream, rot_min_limits);
        METRO_SERIALIZE_MEMBER(stream, rot_max_limits);
    } else {
        METRO_SERIALIZE_MEMBER(stream, constraints);
        if (proceduralVersion >= 6) {
            METRO_SERIALIZE_MEMBER(stream, rot_limits);
        }
    }

    if (proceduralVersion >= 4) {
        METRO_SERIALIZE_MEMBER(stream, use_world_pos);
    }
}

void MetroConstrainedBone::ParentBones::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, axis);
    METRO_SERIALIZE_MEMBER_STRARRAY_CHOOSE(stream, bone_names);

    uint32_t bone_strs_size = scast<uint32_t>(bone_strs.size());
    METRO_SERIALIZE_MEMBER(stream, bone_strs_size);
    if (bone_strs_size > 0) {
        if (stream.IsIn()) {
            bone_strs.resize(bone_strs_size);
        }
        assert(stream.HasDebugInfo() == false);
        for (uint32_t i = 0; i < bone_strs_size; ++i) {
            // if debug info - need to read it
            // "%s%d", "bone", i
            stream >> bone_strs[i].bone;
            // "%s%d", "weight", i
            stream >> bone_strs[i].weight;
        }
    }
}

void MetroConstrainedBone::Serialize(MetroReflectionStream& stream) {
    const uint32_t skeletonVersion = UnpackSkeletonVersion(stream.GetUserData());
    const uint32_t proceduralVersion = UnpackProceduralVersion(stream.GetUserData());

    METRO_SERIALIZE_MEMBER_CHOOSE(stream, bone);

    if (skeletonVersion >= 10) {
        METRO_SERIALIZE_MEMBER(stream, look_at_axis);
        METRO_SERIALIZE_MEMBER(stream, pos_axis);
        METRO_SERIALIZE_MEMBER(stream, rot_axis);
    } else {
        METRO_SERIALIZE_NAMED_MEMBER(stream, look_at_axis, axis);
        pos_axis = scast<uint8_t>(MetroProceduralComponent::None);
        rot_axis = scast<uint8_t>(MetroProceduralComponent::None);
    }

    if (proceduralVersion >= 3) {
        METRO_SERIALIZE_MEMBER(stream, rotation_order);
    } else {
        rotation_order = scast<uint8_t>(MetroProceduralRotationOrder::Default);
    }

    METRO_SERIALIZE_STRUCT_MEMBER(stream, position);
    METRO_SERIALIZE_STRUCT_MEMBER(stream, orientation);

    if (proceduralVersion >= 1) {
        METRO_SERIALIZE_MEMBER(stream, refresh_kids);
    }
    if (proceduralVersion >= 5) {
        METRO_SERIALIZE_MEMBER(stream, use_anim_poses);
    }

    if (proceduralVersion >= 7) {
        if (proceduralVersion > 7) {
            METRO_SERIALIZE_MEMBER(stream, pos_min_limits);
            METRO_SERIALIZE_MEMBER(stream, pos_max_limits);
            METRO_SERIALIZE_MEMBER(stream, rot_min_limits);
            METRO_SERIALIZE_MEMBER(stream, rot_max_limits);
        } else {
            METRO_SERIALIZE_MEMBER(stream, pos_limits);
            METRO_SERIALIZE_MEMBER(stream, rot_limits);
        }

        METRO_SERIALIZE_MEMBER(stream, uptype);
        METRO_SERIALIZE_STRUCT_MEMBER(stream, up);
    }
}

void MetroParamBone::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER_CHOOSE(stream, bone);
    METRO_SERIALIZE_MEMBER_CHOOSE(stream, parent);
    METRO_SERIALIZE_MEMBER_CHOOSE(stream, param);
    METRO_SERIALIZE_MEMBER(stream, component);
}

// PROCEDURAL BONES end

void MetroPartition::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, name);
    METRO_SERIALIZE_ARRAY_MEMBER(stream, infl);
}

void MetroIkChain::Serialize(MetroReflectionStream& stream) {
    const uint32_t skeletonVersion = UnpackSkeletonVersion(stream.GetUserData());

    METRO_SERIALIZE_MEMBER(stream, name);

    if (skeletonVersion <= 11) {
        METRO_SERIALIZE_MEMBER(stream, b0);
        METRO_SERIALIZE_MEMBER(stream, b1);
        METRO_SERIALIZE_MEMBER(stream, b2);
        METRO_SERIALIZE_MEMBER(stream, knee_dir);
    } else {
        METRO_SERIALIZE_MEMBER(stream, upper_limb_bone);
        METRO_SERIALIZE_MEMBER(stream, lower_limb_bone);
        METRO_SERIALIZE_MEMBER(stream, knee_dir);
        METRO_SERIALIZE_MEMBER(stream, max_length);
        METRO_SERIALIZE_MEMBER(stream, flags);

        if (this->flags.value & 0x100) {
            METRO_SERIALIZE_MEMBER(stream, ground_locator);
        }
    }

    METRO_SERIALIZE_MEMBER(stream, knee_lim);
}

void MetroFixedBone::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, id);
}

void MetroSkelParam::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, name);
    METRO_SERIALIZE_MEMBER(stream, b);
    METRO_SERIALIZE_MEMBER(stream, e);
    METRO_SERIALIZE_MEMBER(stream, loop);
}

void MetroWeightedMotion::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, m);
    METRO_SERIALIZE_MEMBER(stream, w);
}

void MotionsCollection::Serialize(MetroReflectionStream& stream) {
    METRO_SERIALIZE_MEMBER(stream, name);
    METRO_SERIALIZE_MEMBER(stream, path);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(stream, mots);
}



MetroSkeleton::MetroSkeleton()
    : ver(0)
    , crc(0)
    , has_as(false)
    , mProceduralVersion(0)
{
}
MetroSkeleton::~MetroSkeleton() {

}

bool MetroSkeleton::LoadFromData(MemStream& stream) {
    bool result = false;

    MetroBinArchive bin(kEmptyString, stream, MetroBinArchive::kHeaderNotExist);
    StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();
    if (reader) {
        this->Serialize(*reader);
        result = !this->bones.empty();
    }

    return result;
}


static quat MetroEulerToQuat(const vec3& r) {
    mat4 glmM = glm::eulerAngleZXY(-r.y, -r.x, -r.z);
    quat q = glm::quat_cast(glmM);
    std::swap(q.y, q.z);
    return QuatConjugate(q);
}

static vec3 QuatToMetroEuler(const quat& q) {
    mat4 mat = glm::mat4_cast(q);
    const mat4::col_type& i = mat[0];
    const mat4::col_type& j = mat[1];
    const mat4::col_type& k = mat[2];

    float x, y, z;

    float cy = Sqrt(j.y * j.y + i.y * i.y);
    if (cy > 16.0f * MM_Epsilon) {
        x = -atan2(k.x, k.z);
        y = -atan2(-k.y, cy);
        z = -atan2(i.y, j.y);
    } else {
        x = -atan2(-i.z, i.x);
        y = -atan2(-k.y, cy);
        z = 0.0f;
    }

    return vec3(-y, -x, -z);
}

bool MetroSkeleton::LoadFromData_2033(MemStream& dataStream) {
    bool result = false;

    StreamChunker chunker(dataStream);
    for (size_t i = 0; i < chunker.GetChunksCount(); ++i) {
        const size_t chunkId = chunker.GetChunkIDByIdx(i);
        MemStream stream = chunker.GetChunkStreamByIdx(i);

        switch (chunkId) {
            case SC2033_Version: {
                this->ver = stream.ReadU32();
            } break;

            case SC2033_Bones: {
                this->crc = stream.ReadU32();
                const size_t numBones = stream.ReadU16();
                this->bones.resize(numBones);

                for (MetroBone& b : this->bones) {
                    b.name = stream.ReadStringZ();
                    b.parent = stream.ReadStringZ();
                    vec3 r;
                    stream.ReadStruct(r);
                    b.q = MetroEulerToQuat(r);
                    stream.ReadStruct(b.t);
                    b.bp = scast<uint8_t>(stream.ReadTyped<uint16_t>() & 0xFF);
#if 0
                    vec3 testR = QuatToMetroEuler(b.q);
                    assert(abs(r.x - testR.x) < 0.00001f);
                    assert(abs(r.y - testR.y) < 0.00001f);
                    assert(abs(r.z - testR.z) < 0.00001f);
#endif
                }
            } break;

            case SC2033_Locators: {
                const size_t numLocators = stream.ReadU16();
                this->locators.resize(numLocators);
                for (MetroLocator& l : this->locators) {
                    l.name = stream.ReadStringZ();
                    l.parent = stream.ReadStringZ();
                    vec3 orientationEuler;
                    stream.ReadStruct(orientationEuler);
                    l.q = MetroEulerToQuat(orientationEuler);
                    stream.ReadStruct(l.t);
                }
            } break;

            case SC2033_Partitions: {
                const size_t numBones = this->bones.size();
                assert(numBones > 0);

                const size_t numPartitions = stream.ReadU16();
                this->partitions.resize(numPartitions);
                for (MetroPartition& p : this->partitions) {
                    p.name = stream.ReadStringZ();
                    p.infl.resize(numBones);
                    stream.ReadToBuffer(p.infl.data(), numBones);
                }
            } break;

            case SC2033_Motions: {
                mMotionsStr = this->motions = stream.ReadStringZ();
            } break;

            case SC2033_IKLocks: {
                const size_t numIkLocks = stream.ReadU16();
                this->ik_locks.resize(numIkLocks);
                for (MetroIkLock& lock : this->ik_locks) {
                    lock.chain_idx = stream.ReadU16();
                    lock.pos_weight = scast<float>(stream.ReadU8()) / 255.0f;
                    lock.quat_weight = scast<float>(stream.ReadU8()) / 255.0f;
                }
            } break;

            case SC2033_FaceFX: {
                this->facefx = stream.ReadStringZ();
            } break;

            case SC2033_Params: {
                const size_t numParams = stream.ReadU16();
                this->params.resize(numParams);
                for (MetroSkelParam& p : this->params) {
                    p.name = stream.ReadStringZ();
                    p.b = stream.ReadF32();
                    p.e = stream.ReadF32();
                    p.loop = stream.ReadF32();
                }
            } break;

            case SC2033_IKChains: {
                const size_t numIkChains = stream.ReadU16();
                this->ik_chains.resize(numIkChains);
                for (MetroIkChain& chain : this->ik_chains) {
                    chain.name = stream.ReadStringZ();
                    chain.b0 = stream.ReadU16();
                    chain.b1 = stream.ReadU16();
                    chain.b2 = stream.ReadU16();
                    stream.ReadStruct(chain.knee_dir);
                    chain.knee_lim = stream.ReadF32();
                }
            } break;
        }

        assert(stream.Remains() == 0);
    }

    result = !this->bones.empty();
    if (result) {
#ifdef _DEBUG
        assert(this->CalcBonesCRC() == this->crc);
#endif

        this->LoadMotions();
    }

    return result;
}

void MetroSkeleton::Save(MemWriteStream& stream) {
    MetroReflectionBinaryWriteStream reflection(stream, MetroReflectionFlags::NoSections);
    this->Serialize(reflection);
}

void MetroSkeleton::Save_2033(MemWriteStream& stream) {
    //#NOTE_SK: keep the original chunks order they are in the game files
    const size_t chunksInOrder[] = {
        SC2033_Version,
        SC2033_Bones,
        SC2033_Locators,
        SC2033_Partitions,
        SC2033_FaceFX,
        SC2033_Motions,
        SC2033_IKChains,
        SC2033_IKLocks,
        SC2033_Params
    };

#define START_CHUNK ChunkWriteHelper chunkHelper(stream, chunkId)

    for (const size_t chunkId : chunksInOrder) {
        switch (chunkId) {
            case SC2033_Version: {
                START_CHUNK;
                stream.WriteU32(kSkeletonVersion2033);
            } break;

            case SC2033_Bones: {
                START_CHUNK;
                stream.WriteU32(this->crc);
                stream.WriteU16(scast<uint16_t>(this->bones.size()));
                for (const MetroBone& b : this->bones) {
                    stream.WriteStringZ(b.name);
                    stream.WriteStringZ(b.parent);

                    vec3 rot = QuatToMetroEuler(b.q);
                    stream.Write(rot);
                    stream.Write(b.t);
                    stream.WriteU16(scast<uint16_t>(b.bp));
                }
            } break;

            case SC2033_Locators: {
                START_CHUNK;
                stream.WriteU16(scast<uint16_t>(this->locators.size()));
                for (const MetroLocator& l : this->locators) {
                    stream.WriteStringZ(l.name);
                    stream.WriteStringZ(l.parent);

                    vec3 rot = QuatToMetroEuler(l.q);
                    stream.Write(rot);
                    stream.Write(l.t);
                }
            } break;

            case SC2033_Partitions: {
                if (!this->partitions.empty()) {
                    START_CHUNK;
                    stream.WriteU16(scast<uint16_t>(this->partitions.size()));
                    for (const MetroPartition& p : this->partitions) {
                        stream.WriteStringZ(p.name);
                        stream.Write(p.infl.data(), p.infl.size());
                    }
                }
            } break;

            case SC2033_Motions: {
                START_CHUNK;
                stream.WriteStringZ(this->motions);
            } break;

            case SC2033_IKLocks: {
                if (!this->ik_locks.empty()) {
                    START_CHUNK;
                    stream.WriteU16(scast<uint16_t>(this->ik_locks.size()));
                    for (const MetroIkLock& lock : this->ik_locks) {
                        stream.WriteU16(lock.chain_idx);
                        stream.WriteU8(scast<uint8_t>(Clamp(lock.pos_weight * 255.0f, 0.0f, 255.0f)));
                        stream.WriteU8(scast<uint8_t>(Clamp(lock.quat_weight * 255.0f, 0.0f, 255.0f)));
                    }
                }
            } break;

            case SC2033_FaceFX: {
                START_CHUNK;
                stream.WriteStringZ(this->facefx);
            } break;

            case SC2033_Params: {
                if (!this->params.empty()) {
                    START_CHUNK;
                    stream.WriteU16(scast<uint16_t>(this->params.size()));
                    for (const MetroSkelParam& p : this->params) {
                        stream.WriteStringZ(p.name);
                        stream.WriteF32(p.b);
                        stream.WriteF32(p.e);
                        stream.WriteF32(p.loop);
                    }
                }
            } break;

            case SC2033_IKChains: {
                if (!this->ik_chains.empty()) {
                    START_CHUNK;
                    stream.WriteU16(scast<uint16_t>(this->ik_chains.size()));
                    for (const MetroIkChain& chain : this->ik_chains) {
                        stream.WriteStringZ(chain.name);
                        stream.WriteU16(chain.b0);
                        stream.WriteU16(chain.b1);
                        stream.WriteU16(chain.b2);
                        stream.Write(chain.knee_dir);
                        stream.WriteF32(chain.knee_lim);
                    }
                }
            } break;
        }
    }

#undef START_CHUNK
}

uint32_t MetroSkeleton::GetBonesCRC() const {
    return this->crc;
}

size_t MetroSkeleton::GetNumBones() const {
    return this->bones.size();
}

size_t MetroSkeleton::GetNumLocators() const {
    return this->locators.size();
}

size_t MetroSkeleton::GetNumAuxBones() const {
    return this->aux_bones.size();
}

size_t MetroSkeleton::GetNumAttachPoints() const {
    return this->GetNumBones() + this->GetNumLocators() + this->GetNumAuxBones();
}

size_t MetroSkeleton::LocatorIdxToAttachPointIdx(const size_t locIdx) const {
    return this->GetNumBones() + locIdx;
}

size_t MetroSkeleton::AuxBoneIdxToAttachPointIdx(const size_t auxIdx) const {
    return this->GetNumBones() + this->GetNumLocators() + auxIdx;
}

size_t MetroSkeleton::AttachPointIdxToLocatorIdx(const size_t attpIdx) const {
    return attpIdx - this->GetNumBones();
}

size_t MetroSkeleton::AttachPointIdxToAuxBoneIdx(const size_t attpIdx) const {
    return attpIdx - this->GetNumLocators() - this->GetNumBones();
}

bool MetroSkeleton::IsAttachPointABone(const size_t attpIdx) const {
    return (attpIdx != kInvalidValue) && (attpIdx < this->bones.size());
}

bool MetroSkeleton::IsAttachPointALocator(const size_t attpIdx) const {
    const size_t skip = this->bones.size();
    return (attpIdx != kInvalidValue) && (attpIdx >= skip && attpIdx < (skip + this->locators.size()));
}

bool MetroSkeleton::IsAttachPointAnAuxBone(const size_t attpIdx) const {
    const size_t skip = this->bones.size() + this->locators.size();
    return (attpIdx != kInvalidValue) && (attpIdx >= skip && attpIdx < (skip + this->aux_bones.size()));
}

const quat& MetroSkeleton::GetBoneRotation(const size_t idx) const {
    if (idx < this->bones.size()) {
        return this->bones[idx].q;
    }

    size_t tryIdx = idx - this->bones.size();
    if (tryIdx < this->locators.size()) {
        return this->locators[tryIdx].q;
    }

    tryIdx -= this->locators.size();
    return this->aux_bones[tryIdx].q;
}

const vec3& MetroSkeleton::GetBonePosition(const size_t idx) const {
    if (idx < this->bones.size()) {
        return this->bones[idx].t;
    }

    size_t tryIdx = idx - this->bones.size();
    if (tryIdx < this->locators.size()) {
        return this->locators[tryIdx].t;
    }

    tryIdx -= this->locators.size();
    return this->aux_bones[tryIdx].t;
}

mat4 MetroSkeleton::GetBoneTransform(const size_t idx) const {
#if 0
    mat4 result = MatFromQuat(this->bones[idx].q);
    result[3] = vec4(this->bones[idx].t, 1.0f);
#else
    const quat& q = this->GetBoneRotation(idx);
    const vec3& t = this->GetBonePosition(idx);

    mat4 result = MatFromQuat(q);
    result[3] = vec4(t, 1.0f);
#endif

    return result;
}

mat4 MetroSkeleton::GetBoneFullTransform(const size_t idx) const {
    const size_t parentIdx = this->GetBoneParentIdx(idx);
    if (parentIdx == kInvalidValue) {
        return this->GetBoneTransform(idx);
    } else {
        return this->GetBoneFullTransform(parentIdx) * this->GetBoneTransform(idx);
    }
}

const mat4& MetroSkeleton::GetBoneFullTransformInv(const size_t idx) const {
    return mInvBindPose[idx];
}

size_t MetroSkeleton::GetBoneParentIdx(const size_t idx) const {
    const CharString& parentName = this->GetBoneParentName(idx);
    return this->FindBone(parentName);
}

const CharString& MetroSkeleton::GetBoneName(const size_t idx) const {
    if (idx < this->bones.size()) {
        return this->bones[idx].name;
    }

    size_t tryIdx = idx - this->bones.size();
    if (tryIdx < this->locators.size()) {
        return this->locators[tryIdx].name;
    }

    tryIdx -= this->locators.size();
    return this->aux_bones[tryIdx].name;
}

const CharString& MetroSkeleton::GetBoneParentName(const size_t idx) const {
    if (idx < this->bones.size()) {
        return this->bones[idx].parent;
    }

    size_t tryIdx = idx - this->bones.size();
    if (tryIdx < this->locators.size()) {
        return this->locators[tryIdx].parent;
    }

    tryIdx -= this->locators.size();
    return this->aux_bones[tryIdx].parent;
}

size_t MetroSkeleton::FindBone(const CharString& name) const {
    size_t result = kInvalidValue, idx = 0;
    for (const MetroBone& b : this->bones) {
        if (b.name == name) {
            result = idx;
            break;
        }
        ++idx;
    }

    if (kInvalidValue == result) {
        for (const MetroLocator& l : this->locators) {
            if (l.name == name) {
                result = idx;
                break;
            }
            ++idx;
        }

        if (kInvalidValue == result) {
            for (const MetroAuxBone& a : this->aux_bones) {
                if (a.name == name) {
                    result = idx;
                    break;
                }
                ++idx;
            }
        }
    }


    return result;
}

const CharString& MetroSkeleton::GetMotionsStr() const {
    return mMotionsStr;
}

void MetroSkeleton::SetMotionsStr(const CharString& str) {
    mMotionsStr = this->motions = str;
}

size_t MetroSkeleton::GetNumMotions() const {
    return mMotions.size();
}

CharString MetroSkeleton::GetMotionName(const size_t idx) const {
    const MetroFSPath& file = mMotions[idx].file;
    const CharString& fileName = MetroContext::Get().GetFilesystem().GetName(file);

    CharString name = fileName.substr(0, fileName.length() - 3);
    return name;
}

const CharString& MetroSkeleton::GetMotionPath(const size_t idx) const {
    return mMotions[idx].path;
}

float MetroSkeleton::GetMotionDuration(const size_t idx) const {
    return scast<float>(mMotions[idx].numFrames) / scast<float>(MetroMotion::kFrameRate);
}

RefPtr<MetroMotion> MetroSkeleton::GetMotion(const size_t idx) {
    RefPtr<MetroMotion> motion = mMotions[idx].motion;

    if (!motion) {
        const CharString& name = this->GetMotionName(idx);
        const MetroFSPath& file = mMotions[idx].file;

        motion = MakeRefPtr<MetroMotion>(name);
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(file);
        assert(stream.Good());

        if (MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033) {
            motion->LoadFromData_2033(stream);
        } else {
            motion->LoadFromData(stream);
        }

        mMotions[idx].motion = motion;
    }

    return motion;
}

const CharString& MetroSkeleton::GetFaceFX() const {
    return this->facefx;
}

void MetroSkeleton::SetFaceFX(const CharString& str) {
    this->facefx = str;
}

size_t MetroSkeleton::GetNumParams() const {
    return this->params.size();
}

const MetroSkelParam& MetroSkeleton::GetSkelParam(const size_t idx) const {
    return this->params[idx];
}

void MetroSkeleton::SetSkelParam(const size_t idx, const MetroSkelParam& param) {
    this->params[idx] = param;
}

void MetroSkeleton::AddSkelParam(const MetroSkelParam& param) {
    this->params.push_back(param);
}

void MetroSkeleton::RemoveSkelParam(const size_t idx) {
    this->params.erase(this->params.begin() + idx);
}

void MetroSkeleton::ReplaceSkelParams(const MyArray<MetroSkelParam>& newParams) {
    this->params = newParams;
}

AABBox MetroSkeleton::CalcBBox() const {
    AABBox result;
    result.Reset();

    const size_t numBones = this->GetNumBones();
    for (size_t i = 0; i < numBones; ++i) {
        mat4 m = this->GetBoneFullTransform(i);
        result.Absorb(vec3(m[3]));
    }

    return result;
}

void MetroSkeleton::SetBones(const MyArray<MetroBone>& newBones) {
    this->bones = newBones;
    this->crc = this->CalcBonesCRC();
    this->CacheMatrices();
}

void MetroSkeleton::SetLocators(const MyArray<MetroLocator>& newLocators) {
    this->locators = newLocators;
    this->CacheMatrices();
}


void MetroSkeleton::Serialize(MetroReflectionStream& stream) {
    MetroReflectionStream* skeletonSection = stream.OpenSection("skeleton");
    if (skeletonSection) {
        METRO_SERIALIZE_MEMBER(*skeletonSection, ver);
        METRO_SERIALIZE_MEMBER(*skeletonSection, crc);

        mProceduralVersion = 0;
        skeletonSection->SetUserData(PackSkeletonVersions(this->ver, mProceduralVersion));

        if (this->ver <= 14) {
            METRO_SERIALIZE_MEMBER(*skeletonSection, facefx);
        }
        if (this->ver >= 17) {
            METRO_SERIALIZE_MEMBER(*skeletonSection, pfnn);
        }
        if (this->ver >= 21) {
            METRO_SERIALIZE_MEMBER(*skeletonSection, has_as);
        }

        METRO_SERIALIZE_MEMBER(*skeletonSection, motions);

        if (this->ver >= 13) {
            METRO_SERIALIZE_MEMBER(*skeletonSection, source_info);
        }
        if (this->ver >= 14) {
            METRO_SERIALIZE_MEMBER(*skeletonSection, parent_skeleton);
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, parent_bone_maps);
        }

        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, bones);

        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, locators);

        if (this->ver >= 6) {
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, aux_bones);
        }

        if (this->ver >= 11) {
            MetroReflectionStream* proceduralSection = skeletonSection->OpenSection("procedural");
            if (proceduralSection) {
                METRO_SERIALIZE_NAMED_MEMBER(*proceduralSection, mProceduralVersion, ver);

                skeletonSection->SetUserData(PackSkeletonVersions(this->ver, mProceduralVersion));

                if (mProceduralVersion > 1) {
                    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*proceduralSection, procedural_bones);
                }
                skeletonSection->CloseSection(proceduralSection);
            }
        }

        if (this->ver >= 7) {
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, driven_bones);
        }

        if (this->ver >= 8) {
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, dynamic_bones);
        }

        if (this->ver >= 9) {
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, constrained_bones);

            if (this->ver >= 20) {
                METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, param_bones);
            }
        }

        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, partitions);
        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, ik_chains);
        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, fixed_bones);
        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, params);
        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonSection, mcolls);

        stream.CloseSection(skeletonSection);
    }
    if (stream.IsIn()) {
        assert(!stream.HasSomeMore());
    }

    //this->MergeParentSkeleton();

    this->CacheMatrices();

    if (stream.IsIn()) {
        mMotionsStr = this->motions;
        this->LoadMotions();
    }
}

void MetroSkeleton::CacheMatrices() {
    const size_t numAttachPoints = this->GetNumAttachPoints();
    mInvBindPose.resize(numAttachPoints);

    for (size_t i = 0; i < numAttachPoints; ++i) {
        mInvBindPose[i] = MatInverse(this->GetBoneFullTransform(i));
    }
}

uint32_t MetroSkeleton::CalcBonesCRC() const {
    Crc32Stream crcStream;
    for (const MetroBone& bone : this->bones) {
        crcStream.Update(bone.name.data(), bone.name.size());
    }
    return crcStream.Finalize();
}

void MetroSkeleton::MergeParentSkeleton() {
    if (!this->parent_skeleton.empty()) {
        StrongPtr<MetroSkeleton> parentSkel;

        CharString parentSkelPath = MetroFileSystem::Paths::MeshesFolder + this->parent_skeleton + ".skeleton.bin";
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileFromPath(parentSkelPath);
        if (stream) {
            parentSkel.reset(new MetroSkeleton());
            if (!parentSkel->LoadFromData(stream)) {
                parentSkel.reset();
            }
        }

        if (parentSkel) {
            for (const ParentMapped& pm : this->parent_bone_maps) {
                const size_t parentIdx = parentSkel->FindBone(pm.parent_bone);
                const size_t selfIdx = this->FindBone(pm.self_bone);
                if (parentIdx != kInvalidValue && selfIdx != kInvalidValue) {
                    //???
                }
            }

            if (!mMotionsStr.empty()) {
                mMotionsStr += ',';
            }
            mMotionsStr += parentSkel->GetMotionsStr();
        }
    }
}

void MetroSkeleton::LoadMotions() {
    if (mMotionsStr.empty()) {
        return;
    }

    const bool is2033 = MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    MyArray<MetroFSPath> motionFiles;

    MyArray<StringView> motionFolders = StrSplitViews(mMotionsStr, ',');
    StringArray motionPaths;
    const CharString& motionsExt = MetroContext::Get().GetMotionExtension();
    for (const StringView& s : motionFolders) {
        CharString fullFolderPath = CharString(MetroFileSystem::Paths::MotionsFolder).append(s).append("\\");

        const auto& files = mfs.FindFilesInFolder(fullFolderPath, motionsExt, false);
        for (const auto& file : files) {
            motionPaths.push_back(fullFolderPath + mfs.GetName(file));
        }

        motionFiles.insert(motionFiles.end(), files.begin(), files.end());
    }

    const size_t numBones = this->GetNumBones();

    mMotions.reserve(motionFiles.size());
    size_t i = 0;
    for (const MetroFSPath& fp : motionFiles) {
        MemStream stream = mfs.OpenFileStream(fp);
        if (stream) {
            MetroMotion motion(kEmptyString);
            const bool loaded = is2033 ? motion.LoadHeader_2033(stream) : motion.LoadHeader(stream);
            if (loaded && motion.GetNumBones() == numBones) {
                mMotions.push_back({ fp, motion.GetNumFrames(), motionPaths[i], nullptr });
            }
        }
        ++i;
    }
}
