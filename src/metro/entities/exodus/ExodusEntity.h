#pragma once
#include "ExodusUObject.h"

struct UEntityStaticParamsExodus : public UObjectStaticParams {
    INHERITED_CLASS(UObjectStaticParams);

    CharString  collision_sound;
    CharString  collision_track;
    uint32_t    collision_interval;
    float       collision_move;
    float       attach_threshold;
    float       attach_armor;

    void Serialize(MetroReflectionStream& s) override;
};

struct InventoryItemStaticParams {
    struct Slot {
        uint32_t    slot;

        void Serialize(MetroReflectionStream& s);
    };

    Slot        slot;
    uint8_t     flags;
    float       control_inertion_factor;
    float       speed_coef;
    float       sens_coef;
    uint32_t    sprint2run_time;
    uint32_t    run2sprint_time;
    uint32_t    slot_max_num;
    uint32_t    keepsakes_count;
    CharString  active_holder_attp;
    CharString  holder_attp;
    CharString  holder_attp1;
    CharString  holder_attp2;
    CharString  active_item_attp;
    CharString  item_attp;
    CharString  active_holder_attp_npc;
    CharString  holder_attp_npc;
    CharString  holder_attp1_npc;
    CharString  holder_attp2_npc;
    CharString  active_item_attp_npc;
    CharString  item_attp_npc;
    uint16_t    ui_tag;

    void Serialize(MetroReflectionStream& s);
};

struct InventoryItemObjectStaticParams : public UEntityStaticParamsExodus {
    INHERITED_CLASS(UEntityStaticParamsExodus);

    InventoryItemStaticParams   inventory_item;
    CharString                  hr_class;
    float                       take_impulse;
    CharString                  take_sound;
    bool                        can_be_taken_as_child;

    void Serialize(MetroReflectionStream& s) override;
};

struct HudItemContainerStaticParams {
    void Serialize(MetroReflectionStream& s);
};

struct UpgradeItemStaticParams : public InventoryItemObjectStaticParams {
    INHERITED_CLASS(InventoryItemObjectStaticParams);

    HudItemContainerStaticParams    container;

    void Serialize(MetroReflectionStream& s) override;
};

struct DeviceUpgradeStaticParams : public UpgradeItemStaticParams {
    INHERITED_CLASS(UpgradeItemStaticParams);

    uint8_t menu_event;

    void Serialize(MetroReflectionStream& s) override;
};

struct PlayerTimerStaticParams : public DeviceUpgradeStaticParams {
    INHERITED_CLASS(DeviceUpgradeStaticParams);
};

struct PlayerTimerHudItemStaticParams {
    float       font_size;
    CharString  font_name;
    ivec4       color;
    ivec4       color_active;
    ivec4       color_time;
    ivec4       color_vs;
    CharString  light_bone;

    void Serialize(MetroReflectionStream& s);
};

struct PlayerTimerHudItemObjectStaticParams : public PlayerTimerStaticParams {
    INHERITED_CLASS(PlayerTimerStaticParams);

    PlayerTimerHudItemStaticParams hud_item;

    void Serialize(MetroReflectionStream& s) override;
};

struct ShootingParticlesData {
    void Serialize(MetroReflectionStream& s);
};

struct ShootingLightData {
    void Serialize(MetroReflectionStream& s);
};

struct ShootingWeaponData {
    void Serialize(MetroReflectionStream& s);
};

struct WeaponItemStaticParams : public UpgradeItemStaticParams {
    INHERITED_CLASS(PlayerTimerStaticParams);

    void Serialize(MetroReflectionStream& s) override;
};

struct HudItemStaticParams {
    void Serialize(MetroReflectionStream& s);
};




// UEntities

struct UEntityExodus : public UObjectEffectExodus {
    INHERITED_CLASS(UObjectEffectExodus);

    void Serialize(MetroReflectionStream& s) override;

    float       health;
    uint32_t    dying_mask;
    uint8_t     physics_flags;
    uint8_t     physics_flags1;
    uint8_t     physics_flags2;

    uint8_t     friend_type;
    uint8_t     reaction_type;
    CharString  fixed_bones;
    float       break_impulse_threshold;
    uint8_t     collisions_group;
    uint8_t     scene_type;
    CharString  break_particles_break;
    CharString  break_particles_death;
    CharString  break_sound_death;
    uint8_t     break_sound_death_ai_type;
    uint64_t    type_mask;
    uint32_t    ph_shell_model_src;
    uint32_t    ph_shell_skltn_src;
    uint32_t    ph_shell_skltn_bcount;
    bool        ph_shell_writed;
    bool        attach_with_joint;
    float       footprint_size;
    float       footprint_power;
};

struct LightParam {
    void Serialize(MetroReflectionStream& s);

    uint8_t     type;
    color4f     color;
    float       brightness;
    float       range_far;
    float       lod_scale;
    vec3        data1;
    vec2        data2;
    float       ibl_gen_radius;
    float       range_near;
    float       source_size;
    anglef      cone;
    float       quality;
    vec3        position;
    vec3        direction;
    vec3        right;
    CharString  color_ca;
    CharString  texture;
    flags8      faces;
    Bool8       light_flags1;
    Bool8       light_flags2;
};

struct FlaresData {
    void Serialize(MetroReflectionStream& s);

    CharString  name;
    CharString  bone;
    uint8_t     axis;
    color4f     cmul;
};

struct UEntityLampExodus : public UEntityExodus {
    INHERITED_CLASS(UEntityExodus);

    void Serialize(MetroReflectionStream& s) override;

    bool        initial_state;
    uint8_t     die_sound_type;
    CharString  die_sound;
    CharString  die_particle;
    CharString  light_main_bone;
    CharString  dark_bone;
    CharString  broken_bone;
    LightParam  main_light;
    bool        color_to_aux;
    bool        sync_color_to_aux;
    uint8_t     secondary_type;
    CharString  secondary_bone;
    float       secondary_power;
    float       secondary_radius;
    bool        secondary_mul_by_ao;
    bool        backlight;
    EntityLink  backlight_ref;
    float       backlight_dist;
    bool        backlight_dynamic;
    bool        backlight_ignore_parents;
    bool        backlight_brightness_compensation;
    float       backlight_force_offset;
    CharString  backlight_ray;
    CharString  backlight_ray_particles;
    bool        backlight_trace_npc_only;
    EntityLink  master;
    FlaresData  flares_data;
};

struct UInventoryItemObjectExodus : public UEntityExodus {
    INHERITED_CLASS(UEntityExodus);

    void Serialize(MetroReflectionStream& s) override;

    // inventory_item
    uint8_t     flags0;
    uint16_t    trade_weight;
    uint8_t     ui_force_slot_id;
    bool        anim_simplification; // ??? which class should this member be in ???
};

struct UUpgradeItemExodus : public UInventoryItemObjectExodus {
    INHERITED_CLASS(UInventoryItemObjectExodus);

    void Serialize(MetroReflectionStream& s) override;

    CharString  upgrade_id;
};

struct UDeviceUpgradeExodus : public UUpgradeItemExodus {
    INHERITED_CLASS(UUpgradeItemExodus);
};

struct UPlayerTimerBaseExodus : public UDeviceUpgradeExodus {
    INHERITED_CLASS(UDeviceUpgradeExodus);
};

struct UPlayerTimerExodus : public UPlayerTimerBaseExodus {
    INHERITED_CLASS(UPlayerTimerBaseExodus);
};

struct UPlayerTimerHudItemObjectExodus : public UPlayerTimerExodus {
    INHERITED_CLASS(UPlayerTimerExodus);
};

struct UWeaponItemExodus : public UUpgradeItemExodus {
    INHERITED_CLASS(UUpgradeItemExodus);

    void Serialize(MetroReflectionStream& s) override;

    bool    vr_attach;
    bool    free_on_level;
};
