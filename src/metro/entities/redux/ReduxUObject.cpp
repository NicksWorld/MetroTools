#include "ReduxUObject.h"
#include "metro/reflection/MetroReflection.h"


// UObjects

void UObjectEffectRedux::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_BASE_CLASS(s);

#if 0
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
#endif
}
