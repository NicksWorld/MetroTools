#include "MetroSkeleton.h"
#include "MetroBinArchive.h"
#include "MetroContext.h"
#include "MetroMotion.h"

#include "reflection/MetroReflection.h"

static const size_t kSkeletonVersionRedux       = 8;    // Latest Steam Redux are this version

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

    void Serialize(MetroReflectionStream& reader) {
        METRO_SERIALIZE_MEMBER(reader, bp);
    }
};


void ParentMapped::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, parent_bone);
    METRO_SERIALIZE_MEMBER(reader, self_bone);
    METRO_SERIALIZE_MEMBER(reader, q);
    METRO_SERIALIZE_MEMBER(reader, t);
    METRO_SERIALIZE_MEMBER(reader, s);
}

void MetroBone::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_MEMBER(reader, parent);
    METRO_SERIALIZE_MEMBER(reader, q);
    METRO_SERIALIZE_MEMBER(reader, t);

    const size_t skeletonVersion = reader.GetUserData();
    if (skeletonVersion > 18) {
        METRO_SERIALIZE_MEMBER(reader, bp);
        METRO_SERIALIZE_MEMBER(reader, bpf);
    } else {
        //#NOTE_SK: using a hack to serialize old (Redux) bones
        ReduxBoneBodyPartHelper helper{ this->bp };
        reader >> helper;

        this->bp = scast<uint8_t>(helper.bp & 0xFF);
    }
}

void MetroLocator::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_MEMBER(reader, parent);
    METRO_SERIALIZE_MEMBER(reader, q);
    METRO_SERIALIZE_MEMBER(reader, t);
    METRO_SERIALIZE_MEMBER(reader, fl);
}

void MetroAuxBone::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_MEMBER(reader, parent);
    METRO_SERIALIZE_MEMBER(reader, q);
    METRO_SERIALIZE_MEMBER(reader, t);
}

void MetroDrivenBone::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, bone);           // choose
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, driver);         // choose
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, driver_parent);  // choose
    METRO_SERIALIZE_MEMBER(reader, component);
    METRO_SERIALIZE_MEMBER(reader, twister);
    METRO_SERIALIZE_MEMBER(reader, value_min);
    METRO_SERIALIZE_MEMBER(reader, value_max);
}

void MetroDynamicBone::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, bone);           // choose
    METRO_SERIALIZE_MEMBER(reader, inertia);
    METRO_SERIALIZE_MEMBER(reader, damping);
    METRO_SERIALIZE_MEMBER(reader, constraints);
}

void MetroPartition::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_ARRAY_MEMBER(reader, infl);
}

void MetroIkChain::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_MEMBER(reader, b0);
    METRO_SERIALIZE_MEMBER(reader, b1);
    METRO_SERIALIZE_MEMBER(reader, b2);
    METRO_SERIALIZE_MEMBER(reader, knee_dir);
    METRO_SERIALIZE_MEMBER(reader, knee_lim);
}

void MetroSkelParam::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_MEMBER(reader, b);
    METRO_SERIALIZE_MEMBER(reader, e);
    METRO_SERIALIZE_MEMBER(reader, loop);
}

void MetroWeightedMotion::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, m);
    METRO_SERIALIZE_MEMBER(reader, w);
}

void MotionsCollection::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_MEMBER(reader, path);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(reader, mots);
}



MetroSkeleton::MetroSkeleton()
    : ver(0)
    , crc(0)
    , has_as(false)
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

bool MetroSkeleton::LoadFromData_2033(MemStream& stream) {
    bool result = false;

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadU32();
        const size_t chunkSize = stream.ReadU32();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

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
                    vec3 orientationEuler;
                    stream.ReadStruct(orientationEuler);
                    b.q = QuatFromEuler(vec3(-orientationEuler.z, -orientationEuler.y, -orientationEuler.x));
                    stream.ReadStruct(b.t);
                    b.bp = scast<uint8_t>(stream.ReadTyped<uint16_t>() & 0xFF);

                    b.t = MetroSwizzle(b.t);
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
                    l.q = QuatFromEuler(vec3(-orientationEuler.z, -orientationEuler.y, -orientationEuler.x));
                    stream.ReadStruct(l.t);
                    l.t = MetroSwizzle(l.t);
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

        stream.SetCursor(chunkEnd);
    }

    result = !this->bones.empty();

    this->LoadMotions();

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
                stream.WriteU32(this->ver);
            } break;

            case SC2033_Bones: {
                START_CHUNK;
                stream.WriteU32(this->crc);
                stream.WriteU16(scast<uint16_t>(this->bones.size()));
                for (const MetroBone& b : this->bones) {
                    stream.WriteStringZ(b.name);
                    stream.WriteStringZ(b.parent);

                    vec3 rot = QuatToEuler(b.q);
                    rot = vec3(-rot.z, -rot.y, -rot.x);
                    vec3 t = MetroSwizzle(b.t);
                    stream.Write(rot);
                    stream.Write(t);
                    stream.WriteU16(scast<uint16_t>(b.bp));
                }
            } break;

            case SC2033_Locators: {
                START_CHUNK;
                stream.WriteU16(scast<uint16_t>(this->locators.size()));
                for (const MetroLocator& l : this->locators) {
                    stream.WriteStringZ(l.name);
                    stream.WriteStringZ(l.parent);

                    vec3 rot = QuatToEuler(l.q);
                    rot = vec3(-rot.z, -rot.y, -rot.x);
                    vec3 t = MetroSwizzle(l.t);
                    stream.Write(rot);
                    stream.Write(t);
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
                stream.WriteStringZ(mMotionsStr);
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
}

size_t MetroSkeleton::GetBonesCRC() const {
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

const size_t MetroSkeleton::GetBoneParentIdx(const size_t idx) const {
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

    return std::move(motion);
}


void MetroSkeleton::Serialize(MetroReflectionStream& reflection) {
    if (reflection.IsOut()) {
        // swizzle back to Metro layout before serializing
        this->SwizzleBones();
    }

    MetroReflectionStream* skeletonReader = reflection.OpenSection("skeleton");
    if (skeletonReader) {
        METRO_SERIALIZE_MEMBER(*skeletonReader, ver);
        METRO_SERIALIZE_MEMBER(*skeletonReader, crc);

        skeletonReader->SetUserData(this->ver);

        if (this->ver < 15) {
            METRO_SERIALIZE_MEMBER(*skeletonReader, facefx);
        } else {
            METRO_SERIALIZE_MEMBER(*skeletonReader, pfnn); // if version > 16
        }
        if (this->ver > 20) {
            METRO_SERIALIZE_MEMBER(*skeletonReader, has_as); // if version > 20
        }
        METRO_SERIALIZE_MEMBER(*skeletonReader, motions);
        if (this->ver > 12) {
            METRO_SERIALIZE_MEMBER(*skeletonReader, source_info); // if version > 12
            if (this->ver > 13) {
                METRO_SERIALIZE_MEMBER(*skeletonReader, parent_skeleton); // if version > 13
                METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, parent_bone_maps); // if version > 13
            }
        }

        //#NOTE_SK: order depends on version ?
        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, bones);
        //#TODO_SK: implement Exodus !!!
        if (this->ver < 20) {
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, locators);
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, aux_bones);

            // Arktika.1:
            // if (ver > 10) {
            //    section  "procedural" {
            //        "procedural_bones" ("rec_%04d")
            //    }
            // }
            // if (ver > 6) {
            //     "driven_bones" ("rec_%04d")
            // }
            // if (ver > 7) {
            //     "dynamic_bones" ("rec_%04d")
            // }
            // if (ver > 8) {
            //     "constrained_bones" ("rec_%04d")
            // }
            // ....
            // "partitions" ("rec_%04d")
            // "ik_chains" ("rec_%04d")
            // "fixed_bones" ("rec_%04d")
            // "params" ("rec_%04d")
            // "mcolls" ("rec_%04d")

            //METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, driven_bones);
            //if (this->ver > 7) {
            //    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, dynamic_bones);
            //}
            //METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, partitions);
            //METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, ik_chains);
            //METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, fixed_bones);
            //METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, params);
            //METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*skeletonReader, mcolls);
        }

        reflection.CloseSection(skeletonReader);
    }

    mMotionsStr = this->motions;

    //this->MergeParentSkeleton();

    //#NOTE_SK: fix-up bones transforms by swizzling them back
    this->SwizzleBones();

    this->CacheMatrices();

    this->LoadMotions();
}

void MetroSkeleton::SwizzleBones() {
    for (auto& b : bones) {
        b.q = MetroSwizzle(b.q);
        b.t = MetroSwizzle(b.t);
    }

    for (auto& l : locators) {
        l.q = MetroSwizzle(l.q);
        l.t = MetroSwizzle(l.t);
    }

    for (auto& a : aux_bones) {
        a.q = MetroSwizzle(a.q);
        a.t = MetroSwizzle(a.t);
    }
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

void MetroSkeleton::CacheMatrices() {
    const size_t numAttachPoints = this->GetNumAttachPoints();
    mInvBindPose.resize(numAttachPoints);

    for (size_t i = 0; i < numAttachPoints; ++i) {
        mInvBindPose[i] = MatInverse(this->GetBoneFullTransform(i));
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
