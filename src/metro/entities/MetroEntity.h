#pragma once
#include "MetroUObject.h"

struct UEntityStaticSarams : public UObjectStaticParams {
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

struct InventoryItemObjectStaticParams : public UEntityStaticSarams {
    INHERITED_CLASS(UObjectStaticParams);

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

struct UEntity : public UObjectEffect {
    INHERITED_CLASS(UObjectEffect);

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

struct EntityLamp : public UEntity {
    INHERITED_CLASS(UEntity);

    void Serialize(MetroReflectionStream& s) override;
};

struct InventoryItemObject : public UEntity {
    INHERITED_CLASS(UEntity);

    void Serialize(MetroReflectionStream& s) override;

    // inventory_item
    uint8_t     flags0;
    uint16_t    trade_weight;
    uint8_t     ui_force_slot_id;
    bool        anim_simplification; // ??? which class should this member be in ???
};

struct UpgradeItem : public InventoryItemObject {
    INHERITED_CLASS(InventoryItemObject);

    void Serialize(MetroReflectionStream& s) override;

    CharString  upgrade_id;
};

struct DeviceUpgrade : public UpgradeItem {
    INHERITED_CLASS(UpgradeItem);
};

struct PlayerTimerBase : public DeviceUpgrade {
    INHERITED_CLASS(DeviceUpgrade);
};

struct PlayerTimer : public PlayerTimerBase {
    INHERITED_CLASS(PlayerTimerBase);
};

struct PlayerTimerHudItemObject : public PlayerTimer {
    INHERITED_CLASS(PlayerTimer);
};

struct WeaponItem : public UpgradeItem {
    INHERITED_CLASS(UpgradeItem);

    void Serialize(MetroReflectionStream& s) override;

    bool    vr_attach;
    bool    free_on_level;
};
