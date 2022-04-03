#include "ExodusUObject.h"
#include "metro/reflection/MetroReflection.h"


// UObjects

void UObjectEffectExodus::Serialize(MetroReflectionStream& s) {
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

void UProxyExodus::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);
    METRO_SERIALIZE_MEMBER(s, slice_count);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, entities);
}

void UObjectEffectMExodus::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

    const size_t version = s.GetUserData();
    if (version >= 47) {
        METRO_SERIALIZE_MEMBER(s, particles_color);
    } else {
        color32u temp;
        METRO_SERIALIZE_NAMED_MEMBER(s, temp, particles_color);
        particles_color = Color32UTo4F(temp);
    }
}

void UHelperTextExodus::Serialize(MetroReflectionStream& s) {
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

void PointLink::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, object);
    METRO_SERIALIZE_MEMBER(s, weight);
}

void UAiPointExodus::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);
    for (int i = 0; i != 4; i++) {
        char buf[10];
        sprintf(buf, "link_%d", i);
        s.SerializeStruct(buf, links[i]);
    }
    METRO_SERIALIZE_MEMBER(s, ai_map);
    METRO_SERIALIZE_MEMBER_CHOOSE(s, cover_group);
}

void PatrolState::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, body_state);
    METRO_SERIALIZE_MEMBER(s, anim_state);
    METRO_SERIALIZE_MEMBER(s, movement_type);
    METRO_SERIALIZE_MEMBER(s, weapon_state);
    METRO_SERIALIZE_MEMBER(s, action);
    METRO_SERIALIZE_MEMBER(s, target);
    METRO_SERIALIZE_MEMBER_FLAGS32(s, flags);
    METRO_SERIALIZE_MEMBER(s, anim_state_approach_speed);
    METRO_SERIALIZE_MEMBER(s, approaching_accel);
}

void UPatrolPointExodus::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);
    METRO_SERIALIZE_MEMBER(s, min_wait_time);
    METRO_SERIALIZE_MEMBER(s, max_wait_time);
    state.Serialize(s);
}
