#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroReflectionStream;

struct UObjectInitData {
    uint32_t    cls;
    uint32_t    static_data_key;
    CharString  att_bone_id;
    uint16_t    id;
    uint16_t    parent_id;
    pose_43T    att_offset;
    bool        att_root;

    void Serialize(MetroReflectionStream& s);
};

struct UObjectStaticParams {
    CharString  caption;
    bool        editable;
    bool        visible_for_ai;
    bool        block_ai_los;
    bool        accept_fast_explosion;
    bool        collideable;
    float       usage_distance;

    // transient
    size_t      version;

    virtual void Serialize(MetroReflectionStream& s);
};

struct UnknownStaticParams : public UObjectStaticParams {
    INHERITED_CLASS(UObjectStaticParams);

    BytesArray unknown;

    void Serialize(MetroReflectionStream& s) override;
};

struct InterestInfo {
    uint16_t    min_importance;
    uint16_t    max_importance;
    uint8_t     interest_type;
    uint16_t    duration;
    float       speed;
    float       distance;
    anglef      max_angle_x;
    anglef      max_angle_y;
    float       angle_coef;

    void Serialize(MetroReflectionStream& s);
};


// UObjects

struct UObject {
    static const size_t kVersionLastLight   = 28;
    static const size_t kVersionRedux       = 30;
    static const size_t kVersionArktika1    = 43;
    static const size_t kVersionExodus      = 49;

    UObject() = default;
    virtual ~UObject() = default;

    virtual void Serialize(MetroReflectionStream& s);
    // it actually returns common_vss**
    virtual bool common_vss() {
        return false;
    }

    CharString      name;               // name thing, implement ???
    Bool8           oflags;
    Bool8           sflags;
    float           cull_distance;
    pose_43         pose;
    CharString      visual;
    uint16_t        dao_val;
    color4f         render_aux_val;
    bool            vs_active;
    uint16_t        spatial_sector;
    uint8_t         qsave_chunk;

    // transient members
    UObjectInitData initData;
    CharString      cls;
    CharString      static_data;

    UObjectStaticParams* static_params;
};

struct UnknownObject : public UObject {
    INHERITED_CLASS(UObject);

    BytesArray unknown;

    void Serialize(MetroReflectionStream& s) override;
};

struct UObjectStatic : public UObject {
    INHERITED_CLASS(UObject);

    uint8_t         flags;
    uint8_t         collision_group;
    InterestInfo    interest;

    void Serialize(MetroReflectionStream& s) override;
};

struct UObjectEffect : public UObject {
    INHERITED_CLASS(UObject);

    void Serialize(MetroReflectionStream& s) override;
    bool common_vss() override {
        return true;
    }

    CharString      startup_animation;
    CharString      bone_part;
    uint16_t        start_frame;
    float           speed;
    Bool8           startup_animation_flags;
    uint8_t         force_looped;
    CharString      sound;
    fp32_q8         sound_volume;
    uint8_t         sound_filter;
    CharString      particles;
    uint8_t         particle_flags;
    InterestInfo    interest;
    StringArray     labels;
};

struct UProxy : public UObject {
    INHERITED_CLASS(UObject);

    void Serialize(MetroReflectionStream& s) override;

    uint16_t            slice_count;
    MyArray<EntityLink> entities;
};

struct UObjectEffectM : public UObjectEffect {
    INHERITED_CLASS(UObjectEffect);

    void Serialize(MetroReflectionStream& s) override;

    color4f particles_color;
};

struct UHelperText : public UObject {
    INHERITED_CLASS(UObject);

    void Serialize(MetroReflectionStream& s) override;

    CharString  text;
    CharString  text_key;
    float       size;
    color4f     color;
    CharString  font;
    Bool8       flags0;
    float       width;
    float       height;
    uint8_t     h_alignment;
    float       display_dist;
};

struct PointLink {
    void Serialize(MetroReflectionStream& s);

    EntityLink object;
    float      weight;
};

struct UAiPoint : public UObject {
    INHERITED_CLASS(UObject);

    void Serialize(MetroReflectionStream& s) override;

    PointLink  links[4];
    flags8     ai_map;
    CharString cover_group;
};

struct PatrolState {
    void Serialize(MetroReflectionStream& s);

    CharString body_state;
    CharString anim_state;
    CharString movement_type;
    CharString weapon_state;
    CharString action;
    EntityLink target;
    uint32_t   flags;
    float      anim_state_approach_speed;
    float      approaching_accel;
};

struct UPatrolPoint : public UAiPoint {
    INHERITED_CLASS(UAiPoint);

    void Serialize(MetroReflectionStream& s) override;

    uint32_t    min_wait_time;
    uint32_t    max_wait_time;
    PatrolState state;
};

using UObjectPtr = StrongPtr<UObject>;
using UObjectRPtr = RefPtr<UObject>;
