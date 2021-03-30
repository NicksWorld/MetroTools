#include "MetroEntityFactory.h"
#include "mycommon.h"

static_assert(CRC32("EFFECT") == 0x46985674);

static UObjectPtr InstantiateUObject(const uint32_t classId) {
    switch (classId) {
        case CRC32("EFFECT"):
            return MakeStrongPtr<UObjectEffect>();

        case CRC32("STATICPROP"):
            return MakeStrongPtr<UObjectStatic>();

        case CRC32("O_ENTITY"):
            return MakeStrongPtr<UEntity>();

        default:
            return MakeStrongPtr<UnknownObject>();
    }
}

UObjectPtr MetroEntityFactory::CreateUObject(const UObjectInitData& initData) {
    UObjectPtr object = InstantiateUObject(initData.cls);

    if (object) {
        object->initData = initData;
    }

    return std::move(object);
}
