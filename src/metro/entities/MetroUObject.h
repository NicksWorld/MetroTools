#pragma once
#include "metro/scripts/MetroScript.h"
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

struct UObject {
    static const size_t kVersionLastLight = 28;
    static const size_t kVersionRedux = 30;
    static const size_t kVersionArktika1 = 43;
    static const size_t kVersionExodus = 49;

    UObject() = default;
    virtual ~UObject() = default;

    virtual void Serialize(MetroReflectionStream& s);
    // it actually returns common_vss**
    virtual bool common_vss() {
        return false;
    }

    using Scripts = MyArray<MetroScript>;
    using ScriptRefs = MyArray<MetroScriptRef>;

    CharString         name;               // name thing, implement ???
    Bool8              oflags;
    Bool8              sflags;
    float              cull_distance;
    pose_43            pose;
    CharString         visual;
    uint16_t           dao_val;
    color4f            render_aux_val;
    Scripts            vss_ver_6;
    bool               vs_active;
    uint16_t           spatial_sector;
    uint8_t            qsave_chunk;
    ScriptRefs         commons_vs;
    ScriptRefs         removed_vs;

    // transient members
    UObjectInitData    initData;
    CharString         cls;
    CharString         static_data;

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


using UObjectPtr = StrongPtr<UObject>;
using UObjectRPtr = RefPtr<UObject>;
