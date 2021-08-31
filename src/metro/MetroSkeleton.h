#pragma once
#include "MetroTypes.h"

class MetroReflectionStream;
class MetroMotion;

struct ParentMapped {   // 48 bytes
    CharString  parent_bone;
    CharString  self_bone;
    quat        q;
    vec3        t;
    vec3        s;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroBone {
    CharString  name;
    CharString  parent;
    quat        q;
    vec3        t;
    uint8_t     bp;
    uint8_t     bpf;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroLocator {
    CharString  name;
    CharString  parent;
    quat        q;
    vec3        t;
    Bool8       fl;     // flags : b0 - editor_type, b1 - skip_fx

    void Serialize(MetroReflectionStream& reader);
};

struct MetroAuxBone {
    CharString  name;
    CharString  parent;
    quat        q;
    vec3        t;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroDrivenBone {
    static const uint8_t component_none     = 0x0;
    static const uint8_t component_axisX    = 0x1;
    static const uint8_t component_axisY    = 0x2;
    static const uint8_t component_axisZ    = 0x3;
    static const uint8_t component_offsetX  = 0x4;
    static const uint8_t component_offsetY  = 0x5;
    static const uint8_t component_offsetZ  = 0x6;

    CharString  bone;           // choose
    CharString  driver;         // choose
    CharString  driver_parent;  // choose
    uint8_t     component;
    CharString  twister;
    float       value_min;
    float       value_max;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroDynamicBone {
    CharString  bone;           // choose
    float       inertia;
    float       damping;
    vec3        constraints;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroPartition {
    CharString  name;
    BytesArray  infl;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroIkLock {
    uint16_t    chain_idx;
    float       pos_weight;
    float       quat_weight;
};

struct MetroIkChain {
    CharString  name;
    uint16_t    b0;
    uint16_t    b1;
    uint16_t    b2;
    vec3        knee_dir;
    float       knee_lim;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroFixedBone {
    void Serialize(MetroReflectionStream&) {}
};

struct MetroSkelParam {
    CharString  name;
    float       b;      // begin
    float       e;      // end
    float       loop;

    void Serialize(MetroReflectionStream& reader);
};

struct MetroWeightedMotion {
    CharString  m;      // motion
    float       w;      // weight

    void Serialize(MetroReflectionStream& reader);
};

struct MotionsCollection {
    CharString                      name;
    CharString                      path;
    MyArray<MetroWeightedMotion>    mots;

    void Serialize(MetroReflectionStream& reader);
};

class MetroSkeleton {
public:
    MetroSkeleton();
    ~MetroSkeleton();

    bool                        LoadFromData(MemStream& stream);
    bool                        LoadFromData_2033(MemStream& stream);

    void                        Save(MemWriteStream& stream);
    void                        Save_2033(MemWriteStream& stream);

    size_t                      GetBonesCRC() const;

    size_t                      GetNumBones() const;
    size_t                      GetNumLocators() const;
    size_t                      GetNumAuxBones() const;
    size_t                      GetNumAttachPoints() const;

    size_t                      LocatorIdxToAttachPointIdx(const size_t locIdx) const;
    size_t                      AuxBoneIdxToAttachPointIdx(const size_t auxIdx) const;
    size_t                      AttachPointIdxToLocatorIdx(const size_t attpIdx) const;
    size_t                      AttachPointIdxToAuxBoneIdx(const size_t attpIdx) const;

    bool                        IsAttachPointABone(const size_t attpIdx) const;
    bool                        IsAttachPointALocator(const size_t attpIdx) const;
    bool                        IsAttachPointAnAuxBone(const size_t attpIdx) const;

    const quat&                 GetBoneRotation(const size_t idx) const;
    const vec3&                 GetBonePosition(const size_t idx) const;
    mat4                        GetBoneTransform(const size_t idx) const;
    mat4                        GetBoneFullTransform(const size_t idx) const;
    const mat4&                 GetBoneFullTransformInv(const size_t idx) const;
    const size_t                GetBoneParentIdx(const size_t idx) const;
    const CharString&           GetBoneName(const size_t idx) const;
    const CharString&           GetBoneParentName(const size_t idx) const;
    size_t                      FindBone(const CharString& name) const;

    const CharString&           GetMotionsStr() const;
    size_t                      GetNumMotions() const;
    CharString                  GetMotionName(const size_t idx) const;
    const CharString&           GetMotionPath(const size_t idx) const;
    float                       GetMotionDuration(const size_t idx) const;
    RefPtr<MetroMotion>         GetMotion(const size_t idx);

    AABBox                      CalcBBox() const;

private:
    void                        Serialize(MetroReflectionStream& reflection);
    void                        SwizzleBones();
    void                        MergeParentSkeleton();
    void                        CacheMatrices();
    void                        LoadMotions();

private:
    uint32_t                    ver;
    uint32_t                    crc;
    CharString                  facefx;
    CharString                  pfnn;
    bool                        has_as;
    CharString                  motions;
    CharString                  source_info;
    CharString                  parent_skeleton;
    MyArray<ParentMapped>       parent_bone_maps;
    MyArray<MetroBone>          bones;
    MyArray<MetroLocator>       locators;
    MyArray<MetroAuxBone>       aux_bones;
    MyArray<MetroDrivenBone>    driven_bones;
    MyArray<MetroDynamicBone>   dynamic_bones;
    MyArray<MetroPartition>     partitions;
    MyArray<MetroIkLock>        ik_locks;
    MyArray<MetroIkChain>       ik_chains;
    MyArray<MetroFixedBone>     fixed_bones;
    MyArray<MetroSkelParam>     params;
    MyArray<MotionsCollection>  mcolls;

    CharString                  mMotionsStr;
    MyArray<mat4>               mInvBindPose;

private:
    struct MotionInfo {
        MetroFSPath         file;
        size_t              numFrames;
        CharString          path;
        RefPtr<MetroMotion> motion;
    };

    MyArray<MotionInfo>         mMotions;
};
