#include "MetroEntityFactory.h"

static UObjectPtr InstantiateUObject(const uint32_t classId) {
    switch (classId) {
        case 0x46985674: // EFFECT
            return MakeStrongPtr<UObjectEffect>();

        case 0x2301c4ef: // STATICPROP
            return MakeStrongPtr<UObjectStatic>();

        case 0x0f10b43b: // O_ENTITY
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
