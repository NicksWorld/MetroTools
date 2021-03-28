#include "MetroMaterialsDatabase.h"
#include "MetroBinArchive.h"
#include "reflection/MetroReflection.h"

void MetroMaterialsCommonOptions::Serialize(MetroReflectionStream& reader) {
    //0
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, tex_name_0);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_width_0);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_height_0);
    //1
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, tex_name_1);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_width_1);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_height_1);
    //2
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, tex_name_2);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_width_2);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_height_2);
    //3
    METRO_SERIALIZE_MEMBER_CHOOSE(reader, tex_name_3);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_width_3);
    METRO_SERIALIZE_MEMBER(reader, tex_frame_height_3);
}

void MetroVehicleMaterial::SurfaceDesc::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, type);
    METRO_SERIALIZE_MEMBER(reader, name);
}

void MetroVehicleMaterial::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(reader, tire_types);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(reader, surface_types);
    //#NOTE_SK: this is same logic as in game
    //          I know it looks weird but what can I do ?
    coefs.resize(tire_types.size() * surface_types.size());
    if (!coefs.empty()) {
        size_t idx = 0;
        MetroReflectionStream* coefsReader = reader.OpenSection("coefs");
        if (coefsReader) {
            for (const auto& st : surface_types) {
                MetroReflectionStream* tireCoefsReader = coefsReader->OpenSection(st.name, true);
                if (tireCoefsReader) {
                    for (const auto& tt : tire_types) {
                        tireCoefsReader->SerializeTypeInfo(tt.name, MetroTypeGetAlias<float>());
                        (*tireCoefsReader) >> coefs[idx++];
                    }

                    coefsReader->CloseSection(tireCoefsReader);
                }
            }

            reader.CloseSection(coefsReader);
        }
    }
}

void MetroMaterial::Serialize(MetroReflectionStream& reader) {
    METRO_SERIALIZE_MEMBER(reader, name);
    METRO_SERIALIZE_MEMBER(reader, ph_friction);
    METRO_SERIALIZE_MEMBER(reader, ph_damping);
    METRO_SERIALIZE_MEMBER(reader, ph_spring);
    METRO_SERIALIZE_MEMBER(reader, cl_dmg_factor);
    METRO_SERIALIZE_MEMBER(reader, vs_transp_factor);
    METRO_SERIALIZE_MEMBER(reader, dbg_color);
    METRO_SERIALIZE_MEMBER(reader, veh_surf_type);
}


MetroMaterialsDatabase::MetroMaterialsDatabase() {

}
MetroMaterialsDatabase::~MetroMaterialsDatabase() {

}

bool MetroMaterialsDatabase::LoadFromData(MemStream& stream) {
    MetroBinArchive bin("materials.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);
    StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();

    METRO_SERIALIZE_STRUCT_MEMBER(*reader, common);
    METRO_SERIALIZE_STRUCT_MEMBER(*reader, vehicle_materials);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*reader, materials);

    return true;
}
