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

    void Serialize(MetroReflectionStream& stream);
};

struct MetroBoneBase {
    CharString  name;
    CharString  parent;
    quat        q;
    vec3        t;

    virtual void Serialize(MetroReflectionStream& stream);
};

struct MetroBone : public MetroBoneBase {
    uint8_t     bp;
    uint8_t     bpf;

    virtual void Serialize(MetroReflectionStream& stream) override;
};

struct MetroLocator : public MetroBoneBase {
    Bool8       fl;     // flags : b0 - editor_type, b1 - skip_fx

    virtual void Serialize(MetroReflectionStream& stream) override;
};

struct MetroAuxBone : public MetroBoneBase {
    // ver >= 16
    Bool8       fl;

    virtual void Serialize(MetroReflectionStream& stream) override;
};

// PROCEDURAL BONES start

enum class MetroProceduralComponent : uint8_t {
    None    = 0x0,
    AxisX   = 0x1,
    AxisY   = 0x2,
    AxisZ   = 0x3,
    OffsetX = 0x4,
    OffsetY = 0x5,
    OffsetZ = 0x6
};

enum class MetroProceduralType : uint8_t {
    Driven             = 0x0,
    Posrot_constrained = 0x1,
    Dynamic            = 0x2,
    Lookat_constrained = 0x3,
    Size               = 0x4
};

enum class MetroProceduralRotationOrder : uint8_t {
    Default = 0x0,
    ZYX     = 0x1,
    _2      = 0x2,
    _3      = 0x3,
    _4      = 0x4,
    _5      = 0x5
};

struct MetroProceduralBone {
    uint16_t    type;           // MetroProceduralType
    uint16_t    index_in_array;

    void Serialize(MetroReflectionStream& stream);
};

struct MetroDrivenBone {
    CharString  bone;           // choose
    CharString  driver;         // choose
    CharString  driver_parent;  // choose
    uint8_t     component;      // MetroProceduralComponent
    CharString  twister;
    float       value_min;
    float       value_max;

    // proceduralVer >= 1
    uint8_t     refresh_kids;
    // proceduralVer >= 5
    bool        use_anim_poses;

    void Serialize(MetroReflectionStream& stream);
};

struct MetroDynamicBone {
    CharString  bone;           // choose
    float       inertia;
    float       damping;

    // if proceduralVer >= 9
    vec3        pos_min_limits;
    vec3        pos_max_limits;
    ang3f       rot_min_limits;
    ang3f       rot_max_limits;
    // end if

    // if proceduralVer < 9
    vec3        constraints;
    //      if proceduralVer >= 6
    ang3f       rot_limits;
    //      end if
    // end if

    // if proceduralVer >= 4
    bool        use_world_pos;
    // end if

    void Serialize(MetroReflectionStream& stream);
};

struct MetroConstrainedBone {
    struct ParentBones {
        struct ParentBone {
            CharString  bone;
            float       weight;
        };

        CharString          bone_names;
        MyArray<ParentBone> bone_strs;
        uint8_t             axis;       // MetroProceduralComponent

        void Serialize(MetroReflectionStream& stream);
    };

    CharString  bone;               // choose
    ParentBones position;
    ParentBones orientation;
    uint16_t    bone_id;
    uint16_t    root_id;
    uint8_t     rotation_order;     // MetroProceduralRotationOrder
    uint8_t     look_at_axis;       // MetroProceduralComponent
    uint8_t     pos_axis;           // MetroProceduralComponent
    uint8_t     rot_axis;           // MetroProceduralComponent
    uint8_t     refresh_kids;
    bool        use_anim_poses;

    // v7
    vec3        pos_limits;
    ang3f       rot_limits;
    // > v7
    vec3        pos_min_limits;
    vec3        pos_max_limits;
    ang3f       rot_min_limits;
    ang3f       rot_max_limits;
    uint8_t     uptype;
    ParentBones up;
    //

    void Serialize(MetroReflectionStream& stream);
};

struct MetroParamBone {
    CharString  bone;               // choose
    CharString  parent;             // choose
    CharString  param;              // choose
    uint8_t     component;          // MetroProceduralComponent

    void Serialize(MetroReflectionStream& stream);
};

// PROCEDURAL BONES end

struct MetroPartition {
    CharString  name;
    BytesArray  infl;

    void Serialize(MetroReflectionStream& stream);
};

struct MetroIkLock {
    uint16_t    chain_idx;
    float       pos_weight;
    float       quat_weight;
};

struct MetroIkChain {
    static const uint32_t FlagForwardKnee           = 0x1;
    static const uint32_t FlagProcedural            = 0x2;
    static const uint32_t FlagFixedKneeDir          = 0x4;
    static const uint32_t FlagGroundAlignDisabled   = 0x8;
    static const uint32_t Flag3D                    = 0x10;
    static const uint32_t FlagKneeDirFromLower      = 0x20;
    static const uint32_t FlagGroundClamp           = 0x40;
    static const uint32_t FlagGroundLocator3D       = 0x80;

    CharString  name;
    uint16_t    b0;
    uint16_t    b1;
    uint16_t    b2;
    vec3        knee_dir;
    float       knee_lim;

    // arktika & exodus
    uint16_t    upper_limb_bone;
    uint16_t    lower_limb_bone;
    float       max_length;
    flags32     flags;
    // if flags & 0x100
    uint16_t    ground_locator;

    void Serialize(MetroReflectionStream& stream);
};

struct MetroFixedBone {
    uint16_t    id;

    void Serialize(MetroReflectionStream& stream);
};

struct MetroSkelParam {
    CharString  name;
    float       b;      // begin
    float       e;      // end
    float       loop;

    void Serialize(MetroReflectionStream& stream);
};

struct MetroWeightedMotion {
    CharString  m;      // motion
    float       w;      // weight

    void Serialize(MetroReflectionStream& stream);
};

struct MotionsCollection {
    CharString                      name;
    CharString                      path;
    MyArray<MetroWeightedMotion>    mots;

    void Serialize(MetroReflectionStream& stream);
};

class MetroSkeleton {
public:
    MetroSkeleton();
    ~MetroSkeleton();

    bool                        LoadFromData(MemStream& stream);
    bool                        LoadFromData_2033(MemStream& stream);

    void                        Save(MemWriteStream& stream);
    void                        Save_2033(MemWriteStream& stream);

    uint32_t                    GetBonesCRC() const;

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

    void                        SetBones(const MyArray<MetroBone>& newBones);
    void                        SetLocators(const MyArray<MetroLocator>& newLocators);

private:
    void                        Serialize(MetroReflectionStream& reflection);
    void                        CacheMatrices();
    uint32_t                    CalcBonesCRC() const;
    void                        MergeParentSkeleton();
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
    // procedural
    MyArray<MetroProceduralBone> procedural_bones;
    MyArray<MetroDrivenBone>    driven_bones;
    MyArray<MetroDynamicBone>   dynamic_bones;
    MyArray<MetroConstrainedBone> constrained_bones;
    MyArray<MetroParamBone>     param_bones;
    //
    MyArray<MetroPartition>     partitions;
    MyArray<MetroIkLock>        ik_locks;
    MyArray<MetroIkChain>       ik_chains;
    MyArray<MetroFixedBone>     fixed_bones;
    MyArray<MetroSkelParam>     params;
    MyArray<MotionsCollection>  mcolls;

    uint32_t                    mProceduralVersion;
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
