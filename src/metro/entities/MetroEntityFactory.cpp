#include "MetroEntityFactory.h"
#include "mycommon.h"

const char* DecodeClassId(uint32_t classId);
void LogUnknownClassId(uint32_t classId);

static_assert(CRC32("EFFECT") == 0x46985674);

#define MATCH_CLASS(cls, type) case CRC32(cls): return MakeStrongPtr<type>()

static UObjectPtr InstantiateUObject(const uint32_t classId) {
    switch (classId) {
        MATCH_CLASS("EFFECT", UObjectEffect);
        MATCH_CLASS("STATICPROP", UObjectStatic);
        MATCH_CLASS("O_ENTITY", UEntity);
        MATCH_CLASS("o_hlamp", EntityLamp);
        MATCH_CLASS("PROXY", Proxy);
        MATCH_CLASS("EFFECTM", UObjectEffectM);
        MATCH_CLASS("O_HELPERTEXT", HelperText);
        MATCH_CLASS("O_AIPOINT", AiPoint);
        MATCH_CLASS("PATROL_POINT", PatrolPoint);

        default:
            LogUnknownClassId(classId);
            return MakeStrongPtr<UnknownObject>();
    }
}

UObjectPtr MetroEntityFactory::CreateUObject(const UObjectInitData& initData) {
    UObjectPtr object = InstantiateUObject(initData.cls);
    object->initData = initData;
    object->cls = DecodeClassId(initData.cls);

    return std::move(object);
}

static const char* clsids[] = {
    "AI_VISION_HELPER_ENTITY",
    "AMEBA",
    "AMMO",
    "ANIM_OBJECT",
    "ANIMSCRIPT",
    "ANOMALY",
    "AQUA_FEMALE",
    "AQUA_MALE_BIG",
    "AQUA_MALE_SMALL",
    "ARAHIND",
    "ARM_DEVICES",
    "BIG_MOTHER",
    "BINOCULARS",
    "BIOMASS_LARGE",
    "BLIND",
    "BOAT",
    "BREAKABLE_ADVANCED",
    "BREAKABLE_ICE",
    "BROADENED_LIAN",
    "BUSH",
    "BUSH_ZONE",
    "CAM_TRACK",
    "CANNIBAL",
    "CATFISH",
    "CHARGER",
    "CLAYMORE_ZONE",
    "CLOCK",
    "COMPASS",
    "COSTUME_UPGRADE",
    "DARK",
    "DARKCHILD",
    "DEER",
    "DEVICE",
    "DEVICE_UPGRADE",
    "DEVICE_UPGRADE_LAMP",
    "DOG",
    "DREZINA_HAND",
    "DREZINA_MOTO",
    "DUMMY_NPC",
    "EFFECT",
    "EFFECTM",
    "FILTER",
    "FLEXIBLE_ENTITY",
    "FLOWER",
    "FLYING_CREATURE",
    "FORCE_FIELD",
    "G_LEVEL",
    "G_PERSIS",
    "GASMASK",
    "GATEWAY",
    "GRIZLY",
    "HANDS_FOR_DREZINA",
    "HARPY",
    "HEAD_SHOT",
    "headquarter",
    "HEAP",
    "HEAP_ZONE",
    "HELSING_ARROW",
    "HELSING_ARROW_BREAKABLE",
    "HUD_MNGR",
    "HUD_OBJECT",
    "HUMAN",
    "HUMANIMAL",
    "KID",
    "KULEMET",
    "LADDER",
    "LIAN",
    "LIBRARIAN",
    "LIGHTER",
    "LIGHTER_VISUAL",
    "LURKER",
    "MAP_PAD_UPGRADE",
    "MECH_ENTITY",
    "MEDKIT",
    "MEDKIT_HEAP",
    "MEDKIT_SPECIAL",
    "METAL_DETECTOR_UPGRADE",
    "MODIFIER",
    "MONSTER",
    "MOTION_SENSOR_UPGRADE",
    "MOUNT_GUN",
    "NIGHTVISION",
    "NOSACH",
    "NOSALIS",
    "NOSALIS_FEMALE",
    "NPC",
    "NPC_FX",
    "O_AIPOINT",
    "O_ANIM_ENTITY",
    "O_BASEZONE",
    "O_BRKBL",
    "O_COVER_LINK",
    "O_DUMMY",
    "O_ENTITY",
    "O_EXPLOSION",
    "O_HELPERTEXT",
    "O_HELPERTEXT_COUNTER",
    "o_hlamp",
    "O_INTEREST",
    "O_PHYSICS",
    "O_SCALING_ENTITY",
    "O_WATERZONE",
    "OBJECT",
    "PARTICLES",
    "PATROL_POINT",
    "PLAYER",
    "PLAYER_MAP",
    "PLAYER_TIMER",
    "PLAYER_TIMER_2033",
    "PLAYER_TIMER_UPGRADE",
    "PLAYERS_BODY",
    "PLAYERS_HANDS",
    "PLAYERS_KNIFE",
    "PROXY",
    "PULSOMETER_UPGRADE",
    "RABBIT",
    "RAGDOLL",
    "RAIL_ENTITY",
    "RAT",
    "s_actor",
    "SCRIPTED_ENTITY",
    "SHIELD",
    "siege_bomb",
    "SIMPLE",
    "SIMPLE_MONSTER",
    "SIMPLE_NPC",
    "SNAKE",
    "SOFT_ENTITY",
    "SOFT_ENTITY_INST",
    "STATICPROP",
    "STATICPROP_BREAKABLE",
    "STATICPROP_MOVABLE",
    "STATION_STAND",
    "STRETCHY_HAND",
    "STRETCHY_HANDLEFT",
    "STRETCHY_MAN",
    "TAPE",
    "TORCH",
    "TORCHLIGHT",
    "TORCHLIGHT_UPGRADABLE",
    "TREADMILL_TILE",
    "TURRET",
    "UPGRADE_TAGS",
    "VEHICLE",
    "VIRTUAL_CAMERA",
    "VIRTUAL_HAND",
    "VIRTUAL_MONITOR",
    "VISOR",
    "VISUALSCRIPT",
    "VISUALSCRIPT_REF",
    "VR_ENTITY",
    "WALLMARK",
    "WATCHMAN",
    "WAVES_EMITTER",
    "WB_WEAPON_HOLDER",
    "WEAPON",
    "WEAPON_2012",
    "WEAPON_ABZAC",
    "WEAPON_AK_74",
    "WEAPON_AK74",
    "WEAPON_AK74_TEST",
    "WEAPON_AKSU",
    "WEAPON_ASHOT",
    "WEAPON_ASHOT_2B",
    "WEAPON_BIGUN",
    "WEAPON_C4_DYNAMITE",
    "WEAPON_CLAYMORE",
    "WEAPON_DAGGER",
    "WEAPON_DECOY",
    "WEAPON_DSHK",
    "WEAPON_DUPLET",
    "WEAPON_DUPLET_2B",
    "WEAPON_DUPLET_3B",
    "WEAPON_DUPLET_4B",
    "WEAPON_DYNAMITE",
    "WEAPON_FAKE",
    "WEAPON_FLAME",
    "WEAPON_FLAME_DYNAMITE",
    "WEAPON_FLAME_GRENADE",
    "WEAPON_FLAMETHROWER",
    "WEAPON_FLARE",
    "WEAPON_GATLING",
    "WEAPON_HELLBREATH",
    "WEAPON_HELLBREATH_SHOCK",
    "WEAPON_HELSING",
    "WEAPON_HOLDER",
    "WEAPON_ITEM",
    "WEAPON_ITEM_AMMO",
    "WEAPON_ITEM_LAMP",
    "WEAPON_ITEM_LAMP_BACKLIGHT",
    "WEAPON_ITEM_LASER",
    "WEAPON_ITEM_MAGAZINE",
    "WEAPON_ITEM_NOOBEGUN",
    "WEAPON_ITEM_NOOBETUBE",
    "WEAPON_ITEM_OPTIC",
    "WEAPON_ITEM_PRESET",
    "WEAPON_ITEM_SILENCER",
    "WEAPON_ITEM_SPEEDLOADER",
    "WEAPON_ITEM_VR",
    "WEAPON_ITEM_VR_ATTACH",
    "WEAPON_KNIVES",
    "WEAPON_LAUNCHER",
    "WEAPON_LAUNCHER_TIME",
    "WEAPON_MACHETA",
    "WEAPON_MAG",
    "WEAPON_MEDVED",
    "WEAPON_NOOB_LAUNCHER",
    "WEAPON_NOOB_SHOTGUN",
    "WEAPON_PADONAG",
    "WEAPON_PREVED",
    "WEAPON_REVOLVER",
    "WEAPON_RPK",
    "WEAPON_SAIGA",
    "WEAPON_SHOTGUN",
    "WEAPON_STICKY_DYNAMITE",
    "WEAPON_TIHAR",
    "WEAPON_TUMAK",
    "WEAPON_UBLUDOK",
    "WEAPON_UBOYNICHEG",
    "WEAPON_VENTIL",
    "WEAPON_VSV",
    "WEAPON_VYHLOP",
    "WEB",
    "WEB_ZONE",
    "WICK_VISUAL",
    "WOMAN",
    "WOMAN_COMBAT",
    "WOMAN_STRIP",
};

MyDict<uint32_t, const char*> CreateClassMap() {
    MyDict<uint32_t, const char*> result;
    for (auto el : clsids)
        result[Hash_CalculateCRC32(el)] = el;
    return result;
}
static const auto classMap = CreateClassMap();

const char* DecodeClassId(uint32_t classId)
{
    auto it = classMap.find(classId);
    return it == classMap.end() ? "" : it->second;
}

void LogUnknownClassId(uint32_t classId)
{
    if (auto it = classMap.find(classId); it == classMap.end())
        LogPrintF(LogLevel::Error, "UObject, unknown classId [%08X]", classId);
    else
        LogPrintF(LogLevel::Warning, "UObject, unhandled classId [%s] [%08X]", it->second, classId);
}
