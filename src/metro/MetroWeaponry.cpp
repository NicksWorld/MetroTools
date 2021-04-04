#include "MetroWeaponry.h"
#include "MetroTypes.h"
#include "MetroBinArrayArchive.h"
#include "MetroBinArchive.h"
#include "MetroContext.h"
#include "reflection/MetroReflection.h"


#include <fstream>
#include "jansson.h"    //TODO: temporarily


void MetroWeaponHandle::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER_CHOOSE(s, upgrades);
    METRO_SERIALIZE_MEMBER(s, anim_track_prefix);
    METRO_SERIALIZE_MEMBER(s, anim_track_suffix);

    upgradesArray = StrSplit(upgrades, ',');
}

void MetroShootingParticlesData::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER_CHOOSE(s, flame_particles_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, flame_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_long_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_long);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_end_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_end);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, shot_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, shell_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, overheat_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, overheat_smoke_particles);
}

void MetroShootingLightData::OptionalPayload::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, type);
    METRO_SERIALIZE_MEMBER(s, color);
    METRO_SERIALIZE_MEMBER(s, brightness);
    METRO_SERIALIZE_MEMBER(s, range_far);
    METRO_SERIALIZE_MEMBER(s, lod_scale);
    METRO_SERIALIZE_MEMBER(s, data1);
    METRO_SERIALIZE_MEMBER(s, data2);
    METRO_SERIALIZE_MEMBER(s, ibl_gen_radius);
    METRO_SERIALIZE_MEMBER(s, range_near);
    METRO_SERIALIZE_MEMBER(s, source_size);
    METRO_SERIALIZE_MEMBER(s, cone);
    METRO_SERIALIZE_MEMBER(s, quality);
    METRO_SERIALIZE_MEMBER(s, position);
    METRO_SERIALIZE_MEMBER(s, direction);
    METRO_SERIALIZE_MEMBER(s, right);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, color_ca);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, texture);
    METRO_SERIALIZE_MEMBER_FLAGS8(s, faces);
    METRO_SERIALIZE_MEMBER(s, light_flags1);
    METRO_SERIALIZE_MEMBER(s, light_flags2);
}

void MetroShootingLightData::Serialize(MetroReflectionStream& s) {
    // optional
    if (s.IsIn()) {
        has_shooting_light = METRO_SERIALIZE_STRUCT_MEMBER(s, shooting_light);
    } else if (has_shooting_light) {
        METRO_SERIALIZE_STRUCT_MEMBER(s, shooting_light);
    }

    METRO_SERIALIZE_MEMBER(s, light_var_color);
    METRO_SERIALIZE_MEMBER(s, light_var_range);
    METRO_SERIALIZE_MEMBER(s, light_time);
    METRO_SERIALIZE_MEMBER(s, light_luminosity);
}

void MetroWeaponModifier::DoubletapParticles::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER_CHOOSE(s, flame_particles_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, flame_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_long_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_long);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_end_hud);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, smoke_particles_end);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, shot_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, shell_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, overheat_particles);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, overheat_smoke_particles);
}

void MetroWeaponModifier::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, weapon_sdata_key);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_shot);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_shot_golden);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_draw);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_holster);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_reload);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_jam);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_jammed_shot);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_deafening);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_echo);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_dbl_echo);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_doubletap);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_no_pressure);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_pump_low);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_pump_normal);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_pump_high);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, snd_charging);
    METRO_SERIALIZE_MEMBER(s, snd_shoot_ai_type);
    METRO_SERIALIZE_MEMBER(s, aim_track_accrue);
    METRO_SERIALIZE_MEMBER(s, aim_track_falloff);
    METRO_SERIALIZE_MEMBER(s, pressure_shot_drop);
    METRO_SERIALIZE_MEMBER(s, pressure_overpump_drop);
    METRO_SERIALIZE_MEMBER(s, pressure_rotation_drop);
    METRO_SERIALIZE_MEMBER(s, pressure_after_shot_increase);
    METRO_SERIALIZE_MEMBER(s, track_barrel_spin_modifier);
    METRO_SERIALIZE_MEMBER(s, track_barrel_spin_max);
    METRO_SERIALIZE_MEMBER(s, spin_sens);
    METRO_SERIALIZE_MEMBER(s, heating_speed);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, steam_particles);

    // !optional
    if (s.IsIn()) {
        has_doubletap_particles = METRO_SERIALIZE_STRUCT_MEMBER(s, doubletap_particles);
    } else if (has_doubletap_particles) {
        METRO_SERIALIZE_STRUCT_MEMBER(s, doubletap_particles);
    }

    METRO_SERIALIZE_MEMBER(s, need_doubletap_particles);
    METRO_SERIALIZE_MEMBER(s, hud);
    METRO_SERIALIZE_MEMBER(s, need_offset);
    METRO_SERIALIZE_MEMBER(s, need_echo);
    METRO_SERIALIZE_MEMBER(s, can_long_idle);
    METRO_SERIALIZE_MEMBER_ANIMSTR(s, jump_idle);
    METRO_SERIALIZE_MEMBER_ANIMSTR(s, long_idle);
    METRO_SERIALIZE_MEMBER_ANIMSTR(s, friend_idle);
    METRO_SERIALIZE_MEMBER_ANIMSTR(s, friend_idle_run);
    METRO_SERIALIZE_MEMBER_ANIMSTR(s, lock_shoot);
    METRO_SERIALIZE_MEMBER_ANIMSTR(s, lock_aim_shoot);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, lock_shoot_track);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, lock_aim_shoot_track);
}

void MetroShootingWeaponData::DirtModifiers::Serialize(MetroReflectionStream& s) {
    const size_t contentVersion = s.GetUserData();

    METRO_SERIALIZE_MEMBER(s, mod_dispersion_base);
    METRO_SERIALIZE_MEMBER(s, mod_dispersion_dec);
    METRO_SERIALIZE_MEMBER(s, mod_blt_power);
    METRO_SERIALIZE_MEMBER(s, mod_blt_impulse);
    METRO_SERIALIZE_MEMBER(s, mod_blt_fire_distance_min);
    METRO_SERIALIZE_MEMBER(s, mod_blt_fire_distance_max);
    METRO_SERIALIZE_MEMBER(s, mod_blt_speed);
    METRO_SERIALIZE_MEMBER(s, mod_rpm);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_decrease_time);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_shake_speed_coef);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_shake_coef);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_horiz_random);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_horiz_coef);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_horiz_coef_pad);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_increase);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_increase_pad);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_vert_coef);
    METRO_SERIALIZE_MEMBER(s, mod_recoil_vert_coef_pad);
    METRO_SERIALIZE_MEMBER(s, mod_wpn_aim_shot_weight);
    METRO_SERIALIZE_MEMBER(s, mod_wpn_shot_weight);
    METRO_SERIALIZE_MEMBER(s, mod_shot_heat_increase);
    METRO_SERIALIZE_MEMBER(s, mod_heat_decrease_speed);
    METRO_SERIALIZE_MEMBER(s, mod_ui_recoil);
    METRO_SERIALIZE_MEMBER(s, mod_ui_damage);
    METRO_SERIALIZE_MEMBER(s, mod_ui_accuracy);
    METRO_SERIALIZE_MEMBER(s, mod_ui_rpm);

    // !optional
    if (contentVersion > 0) {
        METRO_SERIALIZE_MEMBER(s, mod_ui_magazine);
    } else {
        mod_ui_magazine = 1.0f;
    }
}

void MetroShootingWeaponData::AxisDisplacement::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, scale_horizontal);
    METRO_SERIALIZE_MEMBER(s, scale_vertical);
    METRO_SERIALIZE_MEMBER(s, deviation_horizontal);
    METRO_SERIALIZE_MEMBER(s, deviation_vertical);
}

void MetroShootingWeaponData::Serialize(MetroReflectionStream& s) {
    const size_t contentVersion = s.GetUserData();

    METRO_SERIALIZE_MEMBER(s, dispersion_base);
    METRO_SERIALIZE_MEMBER(s, dispersion_base_surv);
    METRO_SERIALIZE_MEMBER(s, dispersion_aim);
    METRO_SERIALIZE_MEMBER(s, dispersion_aim_surv);
    METRO_SERIALIZE_MEMBER(s, dispersion_move_factor);
    METRO_SERIALIZE_MEMBER(s, disp_rot_factor);
    METRO_SERIALIZE_MEMBER(s, dispersion_lh);
    METRO_SERIALIZE_MEMBER(s, dispersion_inc);
    METRO_SERIALIZE_MEMBER(s, dispersion_min);
    METRO_SERIALIZE_MEMBER(s, dispersion_max);
    METRO_SERIALIZE_MEMBER(s, dispersion_max_npc);
    METRO_SERIALIZE_MEMBER(s, dispersion_dec);
    METRO_SERIALIZE_MEMBER(s, min_ammo_show);
    METRO_SERIALIZE_MEMBER(s, num_shot_dispersed);
    METRO_SERIALIZE_MEMBER(s, rpm);

    // DLC2 addition
    if (contentVersion > 0) {
        METRO_SERIALIZE_MEMBER(s, burst_interval);
        METRO_SERIALIZE_MEMBER(s, burst_ammo_count);
    } else {
        burst_interval = 0.0f;
        burst_ammo_count = 0;
    }
    ////////////////

    METRO_SERIALIZE_MEMBER(s, ui_damage);
    METRO_SERIALIZE_MEMBER(s, ui_accuracy);
    METRO_SERIALIZE_MEMBER(s, ui_rpm);

    // DLC2 addition
    if (contentVersion > 0) {
        METRO_SERIALIZE_MEMBER(s, ui_magazine);
    } else {
        ui_magazine = 0.0f;
    }
    ////////////////

    // bullet params
    METRO_SERIALIZE_MEMBER(s, blt_power);
    METRO_SERIALIZE_MEMBER(s, blt_impulse);
    METRO_SERIALIZE_MEMBER(s, blt_power_falloff);
    METRO_SERIALIZE_MEMBER(s, blt_impulse_falloff);
    METRO_SERIALIZE_MEMBER(s, blt_speed);
    METRO_SERIALIZE_MEMBER(s, blt_head_coef);
    METRO_SERIALIZE_MEMBER(s, blt_monster_coef);
    METRO_SERIALIZE_MEMBER(s, blt_fire_distance_min);
    METRO_SERIALIZE_MEMBER(s, blt_fire_distance_max);
    METRO_SERIALIZE_MEMBER(s, blt_fire_distance);
    METRO_SERIALIZE_MEMBER(s, blt_pierce);
    METRO_SERIALIZE_MEMBER(s, blt_gravity_mod);
    METRO_SERIALIZE_MEMBER(s, blt_tracer_scale_xy);
    METRO_SERIALIZE_MEMBER(s, blt_tracer_scale_z);
    METRO_SERIALIZE_MEMBER(s, blt_tracer_min_dist);
    METRO_SERIALIZE_MEMBER(s, blt_tracer_probability);
    METRO_SERIALIZE_MEMBER(s, blt_trail_probability);
    METRO_SERIALIZE_MEMBER(s, blt_material);
    //
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, camera_track_attack_arr);
    // recoil
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_vert);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_horiz);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_shake);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_shot_first_vert);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_shot_burst_vert);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_shot_first_horiz);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_shot_burst_horiz);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_shot_first_shake);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_shot_burst_shake);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_return_vert);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, recoil_curve_return_horiz);
    METRO_SERIALIZE_MEMBER(s, recoil_increase);
    METRO_SERIALIZE_MEMBER(s, recoil_increase_pad);
    METRO_SERIALIZE_MEMBER(s, recoil_increase_time);
    METRO_SERIALIZE_MEMBER(s, recoil_vert_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_vert_coef_pad);
    METRO_SERIALIZE_MEMBER(s, recoil_horiz_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_horiz_coef_pad);
    METRO_SERIALIZE_MEMBER(s, recoil_horiz_random);
    METRO_SERIALIZE_MEMBER(s, recoil_decrease_time);
    METRO_SERIALIZE_MEMBER(s, recoil_shake_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_shake_speed_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_shake_falloff);
    METRO_SERIALIZE_MEMBER(s, recoil_crouch_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_aim_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_one_hand_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_weapon_coef);
    METRO_SERIALIZE_MEMBER(s, recoil_double_shot_coef);
    METRO_SERIALIZE_MEMBER(s, ui_recoil);
    //
    METRO_SERIALIZE_MEMBER(s, shot_heat_increase);
    METRO_SERIALIZE_MEMBER(s, heat_decrease_speed);
    METRO_SERIALIZE_MEMBER(s, critical_heat);
    METRO_SERIALIZE_MEMBER(s, critical_heat_smoke);
    METRO_SERIALIZE_MEMBER(s, sway_crouch_coef);
    METRO_SERIALIZE_MEMBER(s, sway_aim_coef);
    METRO_SERIALIZE_MEMBER(s, sway_one_hand_coef);
    METRO_SERIALIZE_MEMBER(s, sway_increase_coef);
    METRO_SERIALIZE_MEMBER(s, sway_stabilize_speed_coef);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, sway_point_loc_name);

    // !optional
    if (s.IsIn()) {
        has_dirt_modifiers = METRO_SERIALIZE_STRUCT_MEMBER(s, dirt_modifiers);
    } else if (has_dirt_modifiers) {
        METRO_SERIALIZE_STRUCT_MEMBER(s, dirt_modifiers);
    }

    METRO_SERIALIZE_MEMBER(s, shot_weight);
    METRO_SERIALIZE_MEMBER(s, shot_weight_surv);
    METRO_SERIALIZE_MEMBER(s, aim_shot_weight);
    METRO_SERIALIZE_MEMBER(s, aim_shot_weight_surv);

    // !optional
    if (s.IsIn()) {
        has_axis_disp = METRO_SERIALIZE_STRUCT_MEMBER(s, axis_disp);
    } else if (has_axis_disp) {
        METRO_SERIALIZE_STRUCT_MEMBER(s, axis_disp);
    }

    METRO_SERIALIZE_MEMBER(s, track_shot_weight);
    METRO_SERIALIZE_MEMBER(s, aim_track_shot_weight);
    METRO_SERIALIZE_MEMBER(s, magazine_size);
    METRO_SERIALIZE_MEMBER(s, max_bullets_in_barrel);
    METRO_SERIALIZE_MEMBER(s, ammo_to_fire);
    METRO_SERIALIZE_MEMBER(s, barrel_state);
    METRO_SERIALIZE_MEMBER(s, wi_flags);
    METRO_SERIALIZE_MEMBER(s, wi_flags2);
    METRO_SERIALIZE_MEMBER(s, has_auto_fire);
    METRO_SERIALIZE_MEMBER(s, has_regular_barrel_loading);
    METRO_SERIALIZE_MEMBER(s, blend_interrupt_by_fire_allowed);
    METRO_SERIALIZE_MEMBER(s, can_jam);
    METRO_SERIALIZE_MEMBER(s, need_update_ssss);
    METRO_SERIALIZE_MEMBER(s, type_priority);
    METRO_SERIALIZE_MEMBER(s, busy_on_shot_time);
    // hud
    METRO_SERIALIZE_MEMBER(s, hud_deltas_weight);
    METRO_SERIALIZE_MEMBER(s, hud_deltas_weight_aim);
    METRO_SERIALIZE_MEMBER(s, hud_deltas_weight_vert);
    METRO_SERIALIZE_MEMBER(s, hud_deltas_weight_coll_max);
    METRO_SERIALIZE_MEMBER(s, hud_deltas_weight_vert_coll_max);
    METRO_SERIALIZE_MEMBER(s, hud_scale_x_aim);
    METRO_SERIALIZE_MEMBER(s, hud_scale_y_aim);
    // aim
    METRO_SERIALIZE_MEMBER_CHOOSE(s, aim_in_camera_track);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, aim_idle_camera_track);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, aim_out_camera_track);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, aim_idle_excl_camera_track);
    METRO_SERIALIZE_MEMBER(s, aim_track_accrue)
    METRO_SERIALIZE_MEMBER(s, aim_track_falloff);
    //
    METRO_SERIALIZE_MEMBER_CHOOSE(s, track_fire_preparation);
    METRO_SERIALIZE_MEMBER(s, fire_bone);
    METRO_SERIALIZE_MEMBER(s, shell_bone);
    METRO_SERIALIZE_MEMBER(s, armory_holder_front);
    METRO_SERIALIZE_MEMBER(s, armory_holder_back);
    METRO_SERIALIZE_MEMBER(s, hud_scale_base);
    METRO_SERIALIZE_MEMBER(s, static_offset_bone);
    // pumping tracks and pneumo stuff
    METRO_SERIALIZE_MEMBER_CHOOSE(s, track_pumping_low);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, track_pumping_med);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, track_pumping_high);
    METRO_SERIALIZE_MEMBER(s, pressure);
    METRO_SERIALIZE_MEMBER(s, pressure_low);
    METRO_SERIALIZE_MEMBER(s, pressure_normal);
    METRO_SERIALIZE_MEMBER(s, pressure_max);
    METRO_SERIALIZE_MEMBER(s, pressure_auto_pump_raise);
    METRO_SERIALIZE_MEMBER(s, pressure_shot_overdrop);
    METRO_SERIALIZE_MEMBER(s, pressure_shot_overdrop_coef_min);
    METRO_SERIALIZE_MEMBER(s, pressure_shot_overdrop_coef_speed_min);
    METRO_SERIALIZE_MEMBER(s, pressure_low_stat_coef_min);
    METRO_SERIALIZE_MEMBER(s, pressure_low_stat_coef_speed_min);
    METRO_SERIALIZE_MEMBER(s, pneumo_enabled);
    METRO_SERIALIZE_MEMBER(s, has_pressure_drop_on_empty_fire);
    //
    METRO_SERIALIZE_MEMBER(s, customize_id);
    METRO_SERIALIZE_MEMBER(s, breaking_speed);
    METRO_SERIALIZE_MEMBER(s, speed_coef_as);
    METRO_SERIALIZE_MEMBER(s, swap_speed_coef);
    METRO_SERIALIZE_MEMBER(s, speed_coef_rpm);
    METRO_SERIALIZE_MEMBER(s, disp_anim_idle_horiz);
    METRO_SERIALIZE_MEMBER(s, disp_anim_idle_horiz_speed);
    METRO_SERIALIZE_MEMBER(s, disp_anim_idle_vert);
    METRO_SERIALIZE_MEMBER(s, disp_anim_idle_vert_speed);
    METRO_SERIALIZE_MEMBER(s, disp_anim_aim);
    METRO_SERIALIZE_MEMBER(s, disp_anim_aim_speed);
    METRO_SERIALIZE_MEMBER(s, disp_anim_horiz_lh_coef);
    METRO_SERIALIZE_MEMBER(s, disp_anim_vert_lh_coef);
    METRO_SERIALIZE_MEMBER(s, crosshair_size_lh_coef);
    METRO_SERIALIZE_MEMBER(s, pressure_rpm_low);
    METRO_SERIALIZE_MEMBER(s, rpm_low_unmodified);
    METRO_SERIALIZE_MEMBER(s, acceleration);
    METRO_SERIALIZE_MEMBER(s, deceleration);
    METRO_SERIALIZE_MEMBER(s, aim_box_distance);
    METRO_SERIALIZE_MEMBER(s, aim_box_y_offset);
    METRO_SERIALIZE_MEMBER(s, aim_box_buffer_size);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, item_attp_trade);
    METRO_SERIALIZE_MEMBER_STRARRAY_CHOOSE(s, attp_invisible_ui);
    //
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, modifiers);
}

void MetroWeaponMagazineData::AmmoClass::Serialize(MetroReflectionStream&) {
}

void MetroWeaponMagazineData::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, ammo_bone_prefix);
    METRO_SERIALIZE_MEMBER(s, s_ammo_ready_bone_format);
    METRO_SERIALIZE_MEMBER(s, inverse_ammo_order);
    METRO_SERIALIZE_MEMBER(s, always_show_empty_ammo);
    METRO_SERIALIZE_MEMBER_ANIMSTR(s, anim_static_loot);
    METRO_SERIALIZE_MEMBER(s, s_ammo_empty_bone_format);
    METRO_SERIALIZE_MEMBER(s, ammo_mag_blend_time);
    METRO_SERIALIZE_MEMBER(s, max_visible_ammo);
    METRO_SERIALIZE_MEMBER(s, flags);
    METRO_SERIALIZE_MEMBER(s, magazine_pair_tag);
    METRO_SERIALIZE_MEMBER(s, ammo_class);

    MetroReflectionStream* data = s.OpenSection("ammo_class");
    if (data) {
        (*data) >> ammo_class_data;

        s.CloseSection(data);
    }
}


void MetroWeaponAttachmentDesc::Serialize(MetroReflectionStream& s) {
    const size_t contentVersion = s.GetUserData();

    METRO_SERIALIZE_MEMBER(s, type);
    METRO_SERIALIZE_MEMBER(s, upgrade_id);
    METRO_SERIALIZE_MEMBER(s, base_handle);

    // data section
    //#NOTE_SK: probably worth making it a structure and use METRO_SERIALIZE_STRUCT_MEMBER(s, data) ???
    MetroReflectionStream* data = s.OpenSection("data");
    if (data) {
        METRO_SERIALIZE_MEMBER_CHOOSE(*data, shared_weapon_params);
        METRO_SERIALIZE_MEMBER(*data, priority);
        METRO_SERIALIZE_MEMBER(*data, ui_tag_lib);
        METRO_SERIALIZE_ARRAY_16_MEMBER(*data, tags);
        METRO_SERIALIZE_ARRAY_16_MEMBER(*data, excl_tags);
        METRO_SERIALIZE_ARRAY_16_MEMBER(*data, dep_tags);
        METRO_SERIALIZE_ARRAY_16_MEMBER(*data, autoinst_tags);
        METRO_SERIALIZE_MEMBER(*data, is_installed);
        METRO_SERIALIZE_MEMBER(*data, editor_name);
        METRO_SERIALIZE_MEMBER(*data, attach_hud_loc);
        METRO_SERIALIZE_MEMBER(*data, attach_offset);
        METRO_SERIALIZE_MEMBER(*data, attach_to_root);
        METRO_SERIALIZE_MEMBER_CHOOSE(*data, visual);
        if (contentVersion > 0) {
            METRO_SERIALIZE_MEMBER(*data, for_dlc2_only);
        } else {
            for_dlc2_only = false;
        }
        METRO_SERIALIZE_MEMBER(*data, anim_track_prefix);
        METRO_SERIALIZE_MEMBER(*data, anim_track_suffix);

        if (data->IsIn()) {
            hasPayload = data->HasSomeMore();
            if (hasPayload) {
                (*data) >> particles_data;
                (*data) >> light_data;
                (*data) >> shooting_weapon_data;

                data->FlushSectionTail(tail);
                //hasMagazinePayload = data->HasSomeMore();
                //if (hasMagazinePayload) {
                //    (*data) >> magazine_data;
                //}
            }
        } else { // Writing
            if (hasPayload) {
                (*data) >> particles_data;
                (*data) >> light_data;
                (*data) >> shooting_weapon_data;

                data->FlushSectionTail(tail);
                //if (hasMagazinePayload) {
                //    (*data) >> magazine_data;
                //}
            }
        }

        s.CloseSection(data);
    }
}


CharString MetroWeaponry::WeaponHandlesFileName = "weapons_handles_storage.bin";
CharString MetroWeaponry::WeaponAttachesFileName = "attaches_handles_storage.bin";

static const size_t kLatestHandlesVersion = 0;
static const size_t kLatestAttachesVersion = 1;

MetroWeaponry::MetroWeaponry()
    : mHandlesVersion(0)
    , mAttachmentsVersion(0)
{
}

MetroWeaponry::~MetroWeaponry() {
}

bool MetroWeaponry::Initialize() {
    bool result = false;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    MetroFSPath fileHandle = mfs.FindFile(MetroFileSystem::Paths::WeaponryFolder + WeaponHandlesFileName);
    if (fileHandle.IsValid()) {
        MemStream stream = mfs.OpenFileStream(fileHandle);
        if (stream) {
            this->ReadWeaponHandles(stream);
        }
    }

    fileHandle = mfs.FindFile(MetroFileSystem::Paths::WeaponryFolder + WeaponAttachesFileName);
    if (fileHandle.IsValid()) {
        MemStream stream = mfs.OpenFileStream(fileHandle);
        if (stream) {
            this->ReadWeaponAttaches(stream);
        }
    }

    if (!mWeaponHandles.empty() && !mWeaponAttachments.empty()) {
        result = true;

        //CharString s0 = this->SaveWeaponHandlesToJson();
        //CharString s1 = this->SaveWeaponAttachesToJson();

        //std::ofstream jsonFile("wpnatt.json", std::ofstream::binary);
        //jsonFile.write(s1.c_str(), s1.length());
        //jsonFile.flush();
        //jsonFile.close();

        //MemWriteStream strm0;
        //this->WriteWeaponAttaches(strm0);

        //std::ofstream binFile("wpnatt.bin", std::ofstream::binary);
        //binFile.write(rcast<const char*>(strm0.Data()), strm0.GetWrittenBytesCount());
        //binFile.flush();
        //binFile.close();

        //this->ReadWeaponAttachesFromJson(s1);
        //MemWriteStream strm1;
        //this->WriteWeaponAttaches(strm1);
        //std::ofstream binFile2("wpnatt_2.bin", std::ofstream::binary);
        //binFile2.write(rcast<const char*>(strm1.Data()), strm1.GetWrittenBytesCount());
        //binFile2.flush();
        //binFile2.close();
    }

    return result;
}

void MetroWeaponry::ReadWeaponHandles(MemStream& stream) {
    mHandlesVersion = 0;

    size_t numEntries = stream.ReadTyped<uint32_t>();
    if (numEntries == MakeFourcc<'A','V','E','R'>()) {
        mHandlesVersion = stream.ReadTyped<uint16_t>();
        numEntries = stream.ReadTyped<uint32_t>();
    }

    mWeaponHandles.resize(numEntries);
    for (size_t i = 0; i < numEntries; ++i) {
        const size_t idx = stream.ReadTyped<uint32_t>();
        const size_t size = stream.ReadTyped<uint32_t>();

        MemStream subStream = stream.Substream(size);

        MetroWeaponHandle& weaponHandle = mWeaponHandles[i];

        weaponHandle.name = subStream.ReadStringZ();
        const uint8_t flags = subStream.ReadTyped<uint8_t>();

        MetroReflectionBinaryReadStream reader(subStream, flags);
        reader.SetUserData(mHandlesVersion);
        reader >> weaponHandle;

        stream.SkipBytes(size);
    }
}

void MetroWeaponry::ReadWeaponAttaches(MemStream& stream) {
    mAttachmentsVersion = 0;

    size_t numEntries = stream.ReadTyped<uint32_t>();
    if (numEntries == MakeFourcc<'A','V','E','R'>()) {
        mAttachmentsVersion = stream.ReadTyped<uint16_t>();
        numEntries = stream.ReadTyped<uint32_t>();
    }

    mWeaponAttachments.resize(numEntries);
    for (size_t i = 0; i < numEntries; ++i) {
        const size_t idx = stream.ReadTyped<uint32_t>();
        const size_t size = stream.ReadTyped<uint32_t>();

        MemStream subStream = stream.Substream(size);

        MetroWeaponAttachmentDesc& attachment = mWeaponAttachments[i];

        attachment.name = subStream.ReadStringZ();
        const uint8_t flags = subStream.ReadTyped<uint8_t>();

        MetroReflectionBinaryReadStream reader(subStream, flags);
        reader.SetUserData(mAttachmentsVersion);
        reader >> attachment;

        stream.SkipBytes(size);
    }
}

void MetroWeaponry::ReadWeaponAttachesFromJson(const CharString& stream) {
    json_error_t jsonErr = {};
    json_t* json = json_loads(stream.c_str(), JSON_DISABLE_EOF_CHECK, &jsonErr);
    if (json) {
        mWeaponAttachments.clear();

        const char* key;
        json_t* elem;
        json_object_foreach(json, key, elem) {
            mWeaponAttachments.push_back(MetroWeaponAttachmentDesc{});
            MetroWeaponAttachmentDesc& attachment = mWeaponAttachments.back();
            attachment.name = CharString(key);
            MetroReflectionJsonReadStream reader(elem);
            reader.SetUserData(kLatestAttachesVersion);
            reader >> attachment;
        }
    }
}

void MetroWeaponry::WriteWeaponHandles(MemWriteStream& stream) {
    const uint8_t flags = MetroReflectionFlags::None;
    const size_t numEntries = mWeaponHandles.size();

    stream.Write(MakeFourcc<'A', 'V', 'E', 'R'>());
    stream.Write(scast<uint16_t>(kLatestHandlesVersion));
    stream.Write(scast<uint32_t>(numEntries));

    for (size_t i = 0; i < numEntries; ++i) {
        const uint32_t idx = scast<uint32_t>(i);

        MetroWeaponHandle& weaponHandle = mWeaponHandles[i];

        MemWriteStream subStream;

        subStream.WriteStringZ(weaponHandle.name.str);
        subStream.Write(flags);

        MetroReflectionBinaryWriteStream reflStream(subStream, flags);
        reflStream.SetUserData(kLatestHandlesVersion);
        reflStream >> weaponHandle;

        const uint32_t dataSize = scast<uint32_t>(subStream.GetWrittenBytesCount());
        stream.Write(idx);
        stream.Write(dataSize);
        stream.Write(subStream.Data(), subStream.GetWrittenBytesCount());
    }
}

void MetroWeaponry::WriteWeaponAttaches(MemWriteStream& stream) {
    const uint8_t flags = MetroReflectionFlags::HasDebugInfo | MetroReflectionFlags::Editor;
    const size_t numEntries = mWeaponAttachments.size();

    stream.Write(MakeFourcc<'A','V','E','R'>());
    stream.Write(scast<uint16_t>(kLatestAttachesVersion));
    stream.Write(scast<uint32_t>(numEntries));

    for (size_t i = 0; i < numEntries; ++i) {
        const uint32_t idx = scast<uint32_t>(i);

        MetroWeaponAttachmentDesc& attachment = mWeaponAttachments[i];

        MemWriteStream subStream;

        subStream.WriteStringZ(attachment.name.str);
        subStream.Write(flags);

        MetroReflectionBinaryWriteStream reflStream(subStream, flags);
        reflStream.SetUserData(kLatestAttachesVersion);
        reflStream >> attachment;

        const uint32_t dataSize = scast<uint32_t>(subStream.GetWrittenBytesCount());
        stream.Write(idx);
        stream.Write(dataSize);
        stream.Write(subStream.Data(), subStream.GetWrittenBytesCount());
    }
}

CharString MetroWeaponry::SaveWeaponHandlesToJson() {
    MetroReflectionJsonWriteStream stream;
    stream.SetUserData(kLatestHandlesVersion);

    for (size_t i = 0, end = mWeaponHandles.size(); i < end; ++i) {
        MetroWeaponHandle& weaponHandle = mWeaponHandles[i];
        MetroReflectionStream* section = stream.OpenSection(weaponHandle.name.str);
        (*section) >> weaponHandle;
        stream.CloseSection(section);
    }

    return std::move(stream.WriteToString());
}

CharString MetroWeaponry::SaveWeaponAttachesToJson() {
    MetroReflectionJsonWriteStream stream;
    stream.SetUserData(kLatestAttachesVersion);

    for (size_t i = 0, end = mWeaponAttachments.size(); i < end; ++i) {
        MetroWeaponAttachmentDesc& attachment = mWeaponAttachments[i];
        MetroReflectionStream* section = stream.OpenSection(attachment.name.str);
        (*section) >> attachment;
        stream.CloseSection(section);
    }

    return std::move(stream.WriteToString());
}
