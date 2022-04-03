#pragma once
#include "../MetroUObject.h"

class MetroReflectionStream;


// UObjects

struct UObjectEffectRedux : public UObject {
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
