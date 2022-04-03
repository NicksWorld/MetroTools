#pragma once
#include "../MetroUObject.h"

class MetroReflectionStream;


// UObjects

struct UObjectEffectExodus : public UObject {
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

struct UProxyExodus : public UObject {
    INHERITED_CLASS(UObject);

    void Serialize(MetroReflectionStream& s) override;

    uint16_t            slice_count;
    MyArray<EntityLink> entities;
};

struct UObjectEffectMExodus : public UObjectEffectExodus {
    INHERITED_CLASS(UObjectEffectExodus);

    void Serialize(MetroReflectionStream& s) override;

    color4f particles_color;
};

struct UHelperTextExodus : public UObject {
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

struct UAiPointExodus : public UObject {
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

struct UPatrolPointExodus : public UAiPointExodus {
    INHERITED_CLASS(UAiPointExodus);

    void Serialize(MetroReflectionStream& s) override;

    uint32_t    min_wait_time;
    uint32_t    max_wait_time;
    PatrolState state;
};
