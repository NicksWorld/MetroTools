#include "MetroEntity.h"
#include "metro/reflection/MetroReflection.h"

void UEntityStaticSarams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    METRO_SERIALIZE_MEMBER_CHOOSE(s, collision_sound);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, collision_track);
    METRO_SERIALIZE_MEMBER(s, collision_interval);
    METRO_SERIALIZE_MEMBER(s, collision_move);
    METRO_SERIALIZE_MEMBER(s, attach_threshold);
    METRO_SERIALIZE_MEMBER(s, attach_armor);
}

void InventoryItemStaticParams::Slot::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, slot);
}

void InventoryItemStaticParams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_STRUCT_MEMBER(s, slot);

    METRO_SERIALIZE_MEMBER(s, flags);
    METRO_SERIALIZE_MEMBER(s, control_inertion_factor);
    METRO_SERIALIZE_MEMBER(s, speed_coef);
    METRO_SERIALIZE_MEMBER(s, sens_coef);
    METRO_SERIALIZE_MEMBER(s, sprint2run_time);
    METRO_SERIALIZE_MEMBER(s, run2sprint_time);
    METRO_SERIALIZE_MEMBER(s, slot_max_num);
    METRO_SERIALIZE_MEMBER(s, keepsakes_count);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, active_holder_attp);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, holder_attp);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, holder_attp1);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, holder_attp2);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, active_item_attp);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, item_attp);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, active_holder_attp_npc);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, holder_attp_npc);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, holder_attp1_npc);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, holder_attp2_npc);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, active_item_attp_npc);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, item_attp_npc);
    METRO_SERIALIZE_MEMBER(s, ui_tag);
}

void InventoryItemObjectStaticParams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_STRUCT_MEMBER(s, inventory_item);
    //
    METRO_SERIALIZE_BASE_CLASS(s);
    //
    METRO_SERIALIZE_MEMBER_CHOOSE(s, hr_class);
    METRO_SERIALIZE_MEMBER(s, take_impulse);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, take_sound);
    METRO_SERIALIZE_MEMBER(s, can_be_taken_as_child);
}

void HudItemContainerStaticParams::Serialize(MetroReflectionStream&) {
    //#TODO_SK:
}

void UpgradeItemStaticParams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    METRO_SERIALIZE_STRUCT_MEMBER(s, container);
}

void DeviceUpgradeStaticParams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    METRO_SERIALIZE_MEMBER(s, menu_event);
}

void PlayerTimerHudItemStaticParams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, font_size);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, font_name);
    METRO_SERIALIZE_MEMBER(s, color);
    METRO_SERIALIZE_MEMBER(s, color_active);
    METRO_SERIALIZE_MEMBER(s, color_time);
    METRO_SERIALIZE_MEMBER(s, color_vs);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, light_bone);
}

void PlayerTimerHudItemObjectStaticParams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    METRO_SERIALIZE_STRUCT_MEMBER(s, hud_item);
}




// UEntities

void UEntity::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, health);
    METRO_SERIALIZE_MEMBER(s, dying_mask);
    METRO_SERIALIZE_MEMBER(s, physics_flags);
    METRO_SERIALIZE_MEMBER(s, physics_flags1);
    METRO_SERIALIZE_MEMBER(s, physics_flags2);
    //
    METRO_SERIALIZE_BASE_CLASS(s);
    //
    METRO_SERIALIZE_MEMBER(s, friend_type);
    METRO_SERIALIZE_MEMBER(s, reaction_type);
    METRO_SERIALIZE_MEMBER_STRARRAY_CHOOSE(s, fixed_bones);
    METRO_SERIALIZE_MEMBER(s, break_impulse_threshold);
    METRO_SERIALIZE_MEMBER(s, collisions_group);
    METRO_SERIALIZE_MEMBER(s, scene_type);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, break_particles_break);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, break_particles_death);
    METRO_SERIALIZE_MEMBER_SOUNDSTR(s, break_sound_death);
    METRO_SERIALIZE_MEMBER(s, break_sound_death_ai_type);
    METRO_SERIALIZE_MEMBER_FLAGS64(s, type_mask);
    METRO_SERIALIZE_MEMBER(s, ph_shell_model_src);
    METRO_SERIALIZE_MEMBER(s, ph_shell_skltn_src);
    METRO_SERIALIZE_MEMBER(s, ph_shell_skltn_bcount);
    METRO_SERIALIZE_MEMBER(s, ph_shell_writed);
    if (ph_shell_writed) {
        //cfg.ReadSection("physics_shell", [this](Config& cfg) {
        //    cfg.ReadArray("elements", [this](Config& cfg, uint32_t idx) {
        //        cfg.r_u16("root_bid");
        //        cfg.r_fp32("accumulated_impulse");
        //        cfg.r_pose("xform");
        //        cfg.r_vec3f("velocity");
        //        cfg.r_bool("nx_awake");
        //        cfg.ReadArray("shapes", [this](Config& cfg, uint32_t idx) {
        //            cfg.r_u16("bid");
        //        });
        //    });
        //});
        s.SkipSection("physics_shell");
    }
    METRO_SERIALIZE_MEMBER(s, attach_with_joint);
    if (attach_with_joint) {
        //cfg.ReadSection("joint_section", [this](Config& cfg) {
        //    bool enabled = cfg.r_bool("enabled");
        //    auto entity_src = cfg.r_entity_link("entity_src");
        //    auto bone_src = cfg.r_attp_str("bone_src");
        //    auto entity_dst = cfg.r_entity_link("entity_dst");
        //    auto bone_dst = cfg.r_attp_str("bone_dst");
        //    auto pos = cfg.r_vec3f("pos");
        //    auto rot = cfg.r_ang3f("rot");
        //    // g_physics_world->vfptr->load_joint_desc
        //    auto joint_type = cfg.r_u16("joint_type");
        //    cfg.ReadSection("params", [this](Config& cfg) {
        //        cfg.r_bytes(cfg.SectionRemains());
        //    });
        //});
        s.SkipSection("joint_section");
    }
    METRO_SERIALIZE_MEMBER(s, footprint_size);
    METRO_SERIALIZE_MEMBER(s, footprint_power);
}

void EntityLamp::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);
}

void InventoryItemObject::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, flags0);
    METRO_SERIALIZE_MEMBER(s, trade_weight);
    METRO_SERIALIZE_MEMBER(s, ui_force_slot_id);
    //
    METRO_SERIALIZE_BASE_CLASS(s);
    //
    METRO_SERIALIZE_MEMBER(s, anim_simplification);
}

void UpgradeItem::Serialize(MetroReflectionStream& s) {
    //
    METRO_SERIALIZE_BASE_CLASS(s);
    //
    METRO_SERIALIZE_MEMBER(s, upgrade_id);
}

void WeaponItem::Serialize(MetroReflectionStream& s) {
    //
    METRO_SERIALIZE_BASE_CLASS(s);
    //
    METRO_SERIALIZE_MEMBER(s, vr_attach);
    METRO_SERIALIZE_MEMBER(s, free_on_level);
}
