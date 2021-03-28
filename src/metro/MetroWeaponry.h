#pragma once
#include "mycommon.h"
#include "mymath.h"
#include "MetroTypes.h"

class MetroReflectionStream;

struct MetroWeaponHandle {
    HashString  name;

    // serialized
    CharString  upgrades;
    CharString  anim_track_prefix;
    CharString  anim_track_suffix;

    // transient
    StringArray upgradesArray;

    void Serialize(MetroReflectionStream& s);
};

struct MetroShootingParticlesData {
    CharString  flame_particles_hud;
    CharString  flame_particles;
    CharString  smoke_particles_hud;
    CharString  smoke_particles;
    CharString  smoke_particles_long_hud;
    CharString  smoke_particles_long;
    CharString  smoke_particles_end_hud;
    CharString  smoke_particles_end;
    CharString  shot_particles;
    CharString  shell_particles;
    CharString  overheat_particles;
    CharString  overheat_smoke_particles;

    void Serialize(MetroReflectionStream& s);
};

struct MetroShootingLightData {
    struct OptionalPayload {
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
        CharString  color_ca;   // color anim
        CharString  texture;
        flags8      faces;      //#TODO_SK: custom editor type, will use choose for now
        Bool8       light_flags1;
        Bool8       light_flags2;

        void Serialize(MetroReflectionStream& s);
    };

    OptionalPayload shooting_light;

    float           light_var_color;
    float           light_var_range;
    float           light_time;
    float           light_luminosity;

    // transient
    bool            has_shooting_light;

    void Serialize(MetroReflectionStream& s);
};

struct MetroCameraTrackAttack {
    void Serialize(MetroReflectionStream&) {
        assert(false);
    }
};

struct MetroWeaponModifier {
    struct DoubletapParticles {
        CharString      flame_particles_hud;        // choose
        CharString      flame_particles;            // choose
        CharString      smoke_particles_hud;        // choose
        CharString      smoke_particles;            // choose
        CharString      smoke_particles_long_hud;   // choose
        CharString      smoke_particles_long;       // choose
        CharString      smoke_particles_end_hud;    // choose
        CharString      smoke_particles_end;        // choose
        CharString      shot_particles;             // choose
        CharString      shell_particles;            // choose
        CharString      overheat_particles;         // choose
        CharString      overheat_smoke_particles;   // choose

        void Serialize(MetroReflectionStream& s);
    };

    CharString          weapon_sdata_key;
    CharString          snd_shot;                   // choose
    CharString          snd_shot_golden;            // choose
    CharString          snd_draw;                   // choose
    CharString          snd_holster;                // choose
    CharString          snd_reload;                 // choose
    CharString          snd_jam;                    // choose
    CharString          snd_jammed_shot;            // choose
    CharString          snd_deafening;              // choose
    CharString          snd_echo;                   // choose
    CharString          snd_dbl_echo;               // choose
    CharString          snd_doubletap;              // choose
    CharString          snd_no_pressure;            // choose
    CharString          snd_pump_low;               // choose
    CharString          snd_pump_normal;            // choose
    CharString          snd_pump_high;              // choose
    CharString          snd_charging;               // choose
    uint32_t            snd_shoot_ai_type;
    float               aim_track_accrue;
    float               aim_track_falloff;
    float               pressure_shot_drop;
    float               pressure_overpump_drop;
    float               pressure_rotation_drop;
    float               pressure_after_shot_increase;
    float               track_barrel_spin_modifier;
    float               track_barrel_spin_max;
    float               spin_sens;
    float               heating_speed;
    CharString          steam_particles;            // choose

    // !optional
    DoubletapParticles  doubletap_particles;
    //
    bool                need_doubletap_particles;

    pose_43             hud;
    bool                need_offset;
    bool                need_echo;
    bool                can_long_idle;
    CharString          jump_idle;                  // animation_str
    CharString          long_idle;                  // animation_str
    CharString          friend_idle;                // animation_str
    CharString          friend_idle_run;            // animation_str
    CharString          lock_shoot;                 // animation_str
    CharString          lock_aim_shoot;             // animation_str
    CharString          lock_shoot_track;           // choose
    CharString          lock_aim_shoot_track;       // choose


    // transient
    bool                has_doubletap_particles;

    void Serialize(MetroReflectionStream& s);
};

struct MetroShootingWeaponData {
    struct DirtModifiers {
        float       mod_dispersion_base;
        float       mod_dispersion_dec;
        float       mod_blt_power;
        float       mod_blt_impulse;
        float       mod_blt_fire_distance_min;
        float       mod_blt_fire_distance_max;
        float       mod_blt_speed;
        float       mod_rpm;
        float       mod_recoil_decrease_time;
        float       mod_recoil_shake_speed_coef;
        float       mod_recoil_shake_coef;
        float       mod_recoil_horiz_random;
        float       mod_recoil_horiz_coef;
        float       mod_recoil_horiz_coef_pad;
        float       mod_recoil_increase;
        float       mod_recoil_increase_pad;
        float       mod_recoil_vert_coef;
        float       mod_recoil_vert_coef_pad;
        float       mod_wpn_aim_shot_weight;
        float       mod_wpn_shot_weight;
        float       mod_shot_heat_increase;
        float       mod_heat_decrease_speed;
        float       mod_ui_recoil;
        float       mod_ui_damage;
        float       mod_ui_accuracy;
        float       mod_ui_rpm;

        // !optional
        float       mod_ui_magazine;

        void Serialize(MetroReflectionStream& s);
    };

    struct AxisDisplacement {
        float       scale_horizontal;
        float       scale_vertical;
        float       deviation_horizontal;
        float       deviation_vertical;

        void Serialize(MetroReflectionStream& s);
    };

    float           dispersion_base;
    float           dispersion_base_surv;
    float           dispersion_aim;
    float           dispersion_aim_surv;
    float           dispersion_move_factor;
    float           disp_rot_factor;
    float           dispersion_lh;
    float           dispersion_inc;
    float           dispersion_min;
    float           dispersion_max;
    float           dispersion_max_npc;
    float           dispersion_dec;
    uint32_t        min_ammo_show;
    uint32_t        num_shot_dispersed;
    float           rpm;
    //!!DLC2 addition
    float           burst_interval;
    uint8_t         burst_ammo_count;
    ////////////////
    float           ui_damage;
    float           ui_accuracy;
    float           ui_rpm;
    float           ui_magazine;    //!!DLC2 addition
    // bullet params
    float           blt_power;
    float           blt_impulse;
    float           blt_power_falloff;
    float           blt_impulse_falloff;
    float           blt_speed;
    float           blt_head_coef;
    float           blt_monster_coef;
    float           blt_fire_distance_min;
    float           blt_fire_distance_max;
    float           blt_fire_distance;
    float           blt_pierce;
    float           blt_gravity_mod;
    float           blt_tracer_scale_xy;
    float           blt_tracer_scale_z;
    float           blt_tracer_min_dist;
    float           blt_tracer_probability;
    float           blt_trail_probability;
    CharString      blt_material;
    //
    MyArray<MetroCameraTrackAttack> camera_track_attack_arr;
    // recoil
    CharString      recoil_curve_vert;              // choose
    CharString      recoil_curve_horiz;             // choose
    CharString      recoil_curve_shake;             // choose
    CharString      recoil_curve_shot_first_vert;   // choose
    CharString      recoil_curve_shot_burst_vert;   // choose
    CharString      recoil_curve_shot_first_horiz;  // choose
    CharString      recoil_curve_shot_burst_horiz;  // choose
    CharString      recoil_curve_shot_first_shake;  // choose
    CharString      recoil_curve_shot_burst_shake;  // choose
    CharString      recoil_curve_return_vert;       // choose
    CharString      recoil_curve_return_horiz;      // choose
    float           recoil_increase;
    float           recoil_increase_pad;
    float           recoil_increase_time;
    float           recoil_vert_coef;
    float           recoil_vert_coef_pad;
    float           recoil_horiz_coef;
    float           recoil_horiz_coef_pad;
    float           recoil_horiz_random;
    float           recoil_decrease_time;
    float           recoil_shake_coef;
    float           recoil_shake_speed_coef;
    float           recoil_shake_falloff;
    float           recoil_crouch_coef;
    float           recoil_aim_coef;
    float           recoil_one_hand_coef;
    float           recoil_weapon_coef;
    float           recoil_double_shot_coef;
    float           ui_recoil;
    //
    float           shot_heat_increase;
    float           heat_decrease_speed;
    float           critical_heat;
    float           critical_heat_smoke;
    float           sway_crouch_coef;
    float           sway_aim_coef;
    float           sway_one_hand_coef;
    float           sway_increase_coef;
    float           sway_stabilize_speed_coef;
    CharString      sway_point_loc_name;            // choose

    // !optional
    DirtModifiers   dirt_modifiers;
    //

    float           shot_weight;
    float           shot_weight_surv;
    float           aim_shot_weight;
    float           aim_shot_weight_surv;

    // axis_disp section
    AxisDisplacement    axis_disp;
    //

    float           track_shot_weight;
    float           aim_track_shot_weight;
    uint32_t        magazine_size;
    uint8_t         max_bullets_in_barrel;
    uint8_t         ammo_to_fire;
    uint8_t         barrel_state;
    Bool8           wi_flags;
    Bool8           wi_flags2;
    uint8_t         has_auto_fire;
    uint8_t         has_regular_barrel_loading;
    uint8_t         blend_interrupt_by_fire_allowed;
    uint8_t         can_jam;
    uint8_t         need_update_ssss;
    uint8_t         type_priority;
    uint32_t        busy_on_shot_time;
    // hud
    float           hud_deltas_weight;
    float           hud_deltas_weight_aim;
    float           hud_deltas_weight_vert;
    float           hud_deltas_weight_coll_max;
    float           hud_deltas_weight_vert_coll_max;
    float           hud_scale_x_aim;
    float           hud_scale_y_aim;
    // aim
    CharString      aim_in_camera_track;            // choose
    CharString      aim_idle_camera_track;          // choose
    CharString      aim_out_camera_track;           // choose
    CharString      aim_idle_excl_camera_track;     // choose
    float           aim_track_accrue;
    float           aim_track_falloff;
    //
    CharString      track_fire_preparation;         // choose
    CharString      fire_bone;
    CharString      shell_bone;
    CharString      armory_holder_front;
    CharString      armory_holder_back;
    CharString      hud_scale_base;
    CharString      static_offset_bone;
    // pumping tracks and pneumo stuff
    CharString      track_pumping_low;              // choose
    CharString      track_pumping_med;              // choose
    CharString      track_pumping_high;             // choose
    float           pressure;
    float           pressure_low;
    float           pressure_normal;
    float           pressure_max;
    float           pressure_auto_pump_raise;
    float           pressure_shot_overdrop;
    float           pressure_shot_overdrop_coef_min;
    float           pressure_shot_overdrop_coef_speed_min;
    float           pressure_low_stat_coef_min;
    float           pressure_low_stat_coef_speed_min;
    uint8_t         pneumo_enabled;
    uint8_t         has_pressure_drop_on_empty_fire;
    //
    uint8_t         customize_id;
    float           breaking_speed;
    float           speed_coef_as;
    float           swap_speed_coef;
    float           speed_coef_rpm;
    float           disp_anim_idle_horiz;
    float           disp_anim_idle_horiz_speed;
    float           disp_anim_idle_vert;
    float           disp_anim_idle_vert_speed;
    float           disp_anim_aim;
    float           disp_anim_aim_speed;
    float           disp_anim_horiz_lh_coef;
    float           disp_anim_vert_lh_coef;
    float           crosshair_size_lh_coef;
    float           pressure_rpm_low;
    float           rpm_low_unmodified;
    float           acceleration;
    float           deceleration;
    float           aim_box_distance;
    float           aim_box_y_offset;
    vec3            aim_box_buffer_size;
    CharString      item_attp_trade;                // choose
    CharString      attp_invisible_ui;              // choose
    //
    MyArray<MetroWeaponModifier> modifiers;


    // transient
    bool            has_dirt_modifiers;
    bool            has_axis_disp;


    void Serialize(MetroReflectionStream& s);
};

struct MetroWeaponMagazineData {
    struct FragAmmo {
        float           k_power;
        float           k_impulse;
        float           k_power_falloff;
        float           k_impulse_falloff;
        float           k_speed;
        float           k_head_coef;
        float           k_monster_coef;
        float           k_fire_distance_min;
        float           k_fire_distance_max;
        float           k_fire_distance;
        float           k_pierce;
        float           k_gravity_mod;
        float           k_tracer_scale_xy;
        float           k_tracer_scale_z;
        float           k_tracer_min_dist;
        float           k_tracer_probability;
        float           k_trail_probability;
        CharString      k_material;
        CharString      tracer_mesh;                // choose
        CharString      tracer_mesh_hud;            // choose
        CharString      tracer_particles;           // choose
        CharString      tracer_particles_hud;       // choose
        CharString      tracer_particles_ric;       // choose
    };

    struct ExplosionDirSet {
        CharString      particles_ground;           // choose
        CharString      particles_ground_gm;        // choose
        CharString      particles_water;            // choose
        CharString      particles_water_gm;         // choose

        void Serialize(MetroReflectionStream& s);
    };

    struct ExplosiveLight {
        uint8_t         type;
        color4f         color;
        float           brightness;
        float           range_far;
        float           lod_scale;
        vec3            data1;
        vec2            data2;
        float           ibl_gen_radius;
        float           range_near;
        float           source_size;
        anglef          cone;
        float           quality;
        vec3            position;
        vec3            direction;
        vec3            right;
        CharString      color_ca;                   // choose
        CharString      texture;                    // choose
        uint8_t         faces;                      // flags8
        Bool8           light_flags1;
        Bool8           light_flags2;

        void Serialize(MetroReflectionStream& s);
    };

    struct AmmoClass {
        float           k_power;
        float           k_impulse;
        float           k_power_falloff;
        float           k_impulse_falloff;
        float           k_speed;
        float           k_head_coef;
        float           k_monster_coef;
        float           k_fire_distance_min;
        float           k_fire_distance_max;
        float           k_fire_distance;
        float           k_pierce;
        float           k_gravity_mod;
        float           k_tracer_scale_xy;
        float           k_tracer_scale_z;
        float           k_tracer_min_dist;
        float           k_tracer_probability;
        float           k_trail_probability;
        CharString      k_material;
        CharString      tracer_mesh;                // choose
        CharString      tracer_mesh_hud;            // choose
        CharString      tracer_particles;           // choose
        CharString      tracer_particles_hud;       // choose
        CharString      tracer_particles_ric;       // choose
        CharString      tracer_particles_ric_hud;   // choose
        CharString      impact_particles;           // choose
        CharString      trail_particles;            // choose
        CharString      trail_particles_hud;        // choose
        float           trail_min_dist;
        float           wm_size;
        CharString      ammo_material;
        Bool8           flags0;
        uint8_t         explosive;
        float           blast_radius_min;
        float           blast_radius_max;
        float           blast_hit_min;
        float           blast_hit_max;
        float           blast_up_throw_factor;
        float           blast_hit_impulse_factor;
        uint8_t         blast_hit_type;
        uint8_t         blast_raycast_mode;
        uint32_t        delay;
        uint32_t        flame_time;
        uint32_t        flame_interval;
        color4f         light_color;
        float           light_range;
        uint32_t        light_time_max;
        uint32_t        explode_duration_max;
        float           k_disp;
        int32_t         buck_shot;
        uint32_t        type;
        bool            craft_res;
        uint32_t        sound_explode_ai_type;
        ExplosiveLight  explosive_light;
        CharString      coloranim;                  // choose
        CharString      water_coloranim;            // choose
        bool            hide_on_explosion;
        float           light_deviation;
        CharString      sound_explode;              // choose
        CharString      sound_explode_echo;         // choose
        CharString      water_sound;                // choose
        CharString      water_surface_sound;        // choose
        CharString      explode_particles;          // choose
        CharString      water_particles;            // choose
        CharString      water_surface_particles;    // choose
        float           dangerous_radius_coef;
        float           wallmark_ray_dist;
        CharString      explosive_material;
        CharString      track_explosion;            // choose
        float           track_explosion_dist;
        float           track_explosion_target;
        bool            track_explosion_single_inst;
        bool            explosion_dir_addpos;
        float           max_dist_exp_particles;
        uint32_t        max_dist_exp_particles_delay;
        MyArray<ExplosionDirSet>  explosion_dir_sets;
        uint32_t        frags_num;
        float           frags_radius;
        float           frag_hit;
        float           frag_hit_impulse;
        float           frag_speed;
        float           frags_dispersion;

        // struct frag_ammo

        void Serialize(MetroReflectionStream& s);
    };

    CharString      ammo_bone_prefix;
    CharString      s_ammo_ready_bone_format;
    bool            inverse_ammo_order;
    bool            always_show_empty_ammo;
    CharString      anim_static_loot;           // animation_str
    CharString      s_ammo_empty_bone_format;
    float           ammo_mag_blend_time;
    uint8_t         max_visible_ammo;
    Bool8           flags;
    uint16_t        magazine_pair_tag;
    uint32_t        ammo_class;
    AmmoClass       ammo_class_data;

    void Serialize(MetroReflectionStream& s);
};

struct MetroWeaponAttachmentDesc {
    HashString                  name;
    uint32_t                    type;
    uint32_t                    upgrade_id;
    CharString                  base_handle;
    // data section
    CharString                  shared_weapon_params;
    uint8_t                     priority;
    uint16_t                    ui_tag_lib;
    MyArray<uint16_t>           tags;
    MyArray<uint16_t>           excl_tags;
    MyArray<uint16_t>           dep_tags;
    MyArray<uint16_t>           autoinst_tags;
    bool                        is_installed;
    CharString                  editor_name;
    CharString                  attach_hud_loc;
    pose_43T                    attach_offset;
    bool                        attach_to_root;
    CharString                  visual;
    bool                        for_dlc2_only;      //#NOTE_SK: this flag was added in DLC2, need to check content version!!!
    CharString                  anim_track_prefix;
    CharString                  anim_track_suffix;

    bool                        hasPayload;
    bool                        hasMagazinePayload;

    // Payload
    MetroShootingParticlesData  particles_data;
    MetroShootingLightData      light_data;
    MetroShootingWeaponData     shooting_weapon_data;
    // Payload magazine ?
    MetroWeaponMagazineData     magazine_data;
    BytesArray                  tail;

    void Serialize(MetroReflectionStream& s);
};

class MetroWeaponry {
    IMPL_SINGLETON(MetroWeaponry)

public:
    static CharString WeaponHandlesFileName;
    static CharString WeaponAttachesFileName;

protected:
    MetroWeaponry();
    ~MetroWeaponry();

public:
    bool        Initialize();

    void        ReadWeaponHandles(MemStream& stream);
    void        ReadWeaponAttaches(MemStream& stream);
    void        ReadWeaponAttachesFromJson(const CharString& stream);

    void        WriteWeaponHandles(MemWriteStream& stream);
    void        WriteWeaponAttaches(MemWriteStream& stream);

    CharString  SaveWeaponHandlesToJson();
    CharString  SaveWeaponAttachesToJson();

private:
    size_t                              mHandlesVersion;
    size_t                              mAttachmentsVersion;
    MyArray<MetroWeaponHandle>          mWeaponHandles;
    MyArray<MetroWeaponAttachmentDesc>  mWeaponAttachments;
};

