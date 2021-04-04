#include "MetroUObject.h"
#include "metro/reflection/MetroReflection.h"


void UObjectInitData::Serialize(MetroReflectionStream& s) {
    const size_t entitiesVersion = s.GetUserData();

    METRO_SERIALIZE_NAMED_MEMBER(s, cls, class);
    METRO_SERIALIZE_MEMBER(s, static_data_key);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, att_bone_id);
    METRO_SERIALIZE_MEMBER(s, id);
    METRO_SERIALIZE_MEMBER(s, parent_id);
    METRO_SERIALIZE_MEMBER(s, att_offset);

    if (entitiesVersion >= UObject::kVersionArktika1) {
        METRO_SERIALIZE_MEMBER(s, att_root);
    } else {
        att_root = false;
    }
}


void UObjectStaticParams::Serialize(MetroReflectionStream& s) {
    this->version = s.GetUserData();

    //#TODO_SK: make proper struct member ???
    MetroReflectionStream* startupReader = s.OpenSection("__edit");
    if (startupReader) {
        METRO_SERIALIZE_MEMBER(*startupReader, caption);
        s.CloseSection(startupReader);
    }

    METRO_SERIALIZE_MEMBER(s, editable);
    METRO_SERIALIZE_MEMBER(s, visible_for_ai);
    METRO_SERIALIZE_MEMBER(s, block_ai_los);
    METRO_SERIALIZE_MEMBER(s, accept_fast_explosion);
    METRO_SERIALIZE_MEMBER(s, collideable);
    METRO_SERIALIZE_MEMBER(s, usage_distance);
}

void UnknownStaticParams::Serialize(MetroReflectionStream& s) {
    this->version = s.GetUserData();

    s.FlushSectionTail(this->unknown);
}

void InterestInfo::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, min_importance);
    METRO_SERIALIZE_MEMBER(s, max_importance);
    METRO_SERIALIZE_MEMBER(s, interest_type);
    METRO_SERIALIZE_MEMBER(s, duration);
    METRO_SERIALIZE_MEMBER(s, speed);
    METRO_SERIALIZE_MEMBER(s, distance);
    METRO_SERIALIZE_MEMBER(s, max_angle_x);
    METRO_SERIALIZE_MEMBER(s, max_angle_y);
    METRO_SERIALIZE_MEMBER(s, angle_coef);
}



// UObjects

void UObject::Serialize(MetroReflectionStream& s) {
    const size_t entitiesVersion = s.GetUserData();

    METRO_SERIALIZE_MEMBER_CHOOSE(s, name);
    METRO_SERIALIZE_MEMBER(s, oflags);
    METRO_SERIALIZE_MEMBER(s, sflags);
    if (entitiesVersion >= UObject::kVersionArktika1) {
        METRO_SERIALIZE_MEMBER(s, cull_distance);
    } else {
        cull_distance = 10000.0f;
    }
    METRO_SERIALIZE_MEMBER(s, pose);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, visual);
    METRO_SERIALIZE_MEMBER(s, dao_val);
    if (entitiesVersion >= UObject::kVersionRedux) {
        METRO_SERIALIZE_MEMBER(s, render_aux_val);
    }

    //#TODO_SK:  METRO_SERIALIZE_STRUCT_ARRAY_MEMBER  !!!
    s.SkipSection("vss_ver_6");

    METRO_SERIALIZE_MEMBER(s, vs_active);
    METRO_SERIALIZE_MEMBER(s, spatial_sector);
    METRO_SERIALIZE_MEMBER(s, qsave_chunk);

    if (this->common_vss()) {
        s.SkipSection("commons_vs");
        s.SkipSection("removed_vs");
    }
}


void UnknownObject::Serialize(MetroReflectionStream& s) {
    s.FlushSectionTail(this->unknown);
}

void UObjectStatic::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    METRO_SERIALIZE_MEMBER(s, flags);
    METRO_SERIALIZE_MEMBER(s, collision_group);
    METRO_SERIALIZE_STRUCT_MEMBER(s, interest);
}

void UObjectEffect::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    METRO_SERIALIZE_MEMBER_ANIMSTR(s, startup_animation);
    METRO_SERIALIZE_MEMBER_PARTSTR(s, bone_part);
    METRO_SERIALIZE_MEMBER(s, start_frame);
    METRO_SERIALIZE_MEMBER(s, speed);
    METRO_SERIALIZE_MEMBER(s, startup_animation_flags);
    METRO_SERIALIZE_MEMBER(s, force_looped);
    METRO_SERIALIZE_MEMBER_SOUNDSTR(s, sound);
    METRO_SERIALIZE_MEMBER(s, sound_volume);
    METRO_SERIALIZE_MEMBER(s, sound_filter);
    METRO_SERIALIZE_MEMBER(s, particle_flags);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, particles);
    METRO_SERIALIZE_STRUCT_MEMBER(s, interest);
    METRO_SERIALIZE_STR_ARRAY_MEMBER(s, labels);
}

void Proxy::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);
    METRO_SERIALIZE_MEMBER(s, slice_count);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, entities);
}

void UObjectEffectM::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    const size_t version = s.GetUserData();
    if (version >= 47) {
        METRO_SERIALIZE_MEMBER(s, particles_color);
    }
    else {
        color32u temp;
        METRO_SERIALIZE_NAMED_MEMBER(s, temp, particles_color);
        // convert color32u to particles_color
    }
}

void HelperText::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    METRO_SERIALIZE_MEMBER(s, text);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, text_key);
    METRO_SERIALIZE_MEMBER(s, size);
    METRO_SERIALIZE_MEMBER(s, color);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, font);
    METRO_SERIALIZE_MEMBER(s, flags0);
    METRO_SERIALIZE_MEMBER(s, width);
    METRO_SERIALIZE_MEMBER(s, height);
    METRO_SERIALIZE_MEMBER(s, h_alignment);
    METRO_SERIALIZE_MEMBER(s, display_dist);
}