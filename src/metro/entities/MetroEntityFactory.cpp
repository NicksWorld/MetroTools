#include "MetroEntityFactory.h"
#include "metro/MetroTypedStrings.h"
#include "mycommon.h"

void LogUnknownClassId(uint32_t classId);

static_assert(CRC32("EFFECT") == 0x46985674);

MetroEntityFactory::MetroEntityFactory() {
    // register all classes !!!
#define REGISTER_UCLASS(cls, type)  this->RegisterClass<type>(CRC32(cls))

    REGISTER_UCLASS("EFFECT",       UObjectEffect);
    REGISTER_UCLASS("STATICPROP",   UObjectStatic);
    REGISTER_UCLASS("O_ENTITY",     UEntity);
    REGISTER_UCLASS("o_hlamp",      UEntityLamp);
    REGISTER_UCLASS("PROXY",        UProxy);
    REGISTER_UCLASS("EFFECTM",      UObjectEffectM);
    REGISTER_UCLASS("O_HELPERTEXT", UHelperText);
    REGISTER_UCLASS("O_AIPOINT",    UAiPoint);
    REGISTER_UCLASS("PATROL_POINT", UPatrolPoint);

#undef REGISTER_UCLASS
}

UObjectPtr MetroEntityFactory::CreateUObject(const UObjectInitData& initData) {
    UObjectPtr object;

    auto it = mRegisteredClasses.find(initData.cls);
    if (it != mRegisteredClasses.end()) {
        object = it->second();
        object->initData = initData;
        object->cls = MetroTypedStrings::Get().GetString(initData.cls).str;
    } else {
        LogUnknownClassId(initData.cls);
        object = MakeStrongPtr<UnknownObject>();
    }

    return std::move(object);
}

void LogUnknownClassId(uint32_t classId)
{
    const auto& s = MetroTypedStrings::Get().GetString(classId);
    if (!s.Valid()) {
        LogPrintF(LogLevel::Error, "UObject, unknown classId [%08X]", classId);
    } else {
        LogPrintF(LogLevel::Warning, "UObject, unhandled classId [%s] [%08X]", s.str.c_str(), classId);
    }
}
