#include "ReduxEntityFactory.h"
#include "ReduxEntity.h"
#include "metro/MetroTypedStrings.h"


static_assert(CRC32("EFFECT") == 0x46985674, "Types crc32 check failed!");

struct TypesStaticRegistrarRedux {
private:
    template <typename T>
    inline void RegisterClass(const uint32_t clsid) {
        registeredClasses.insert({
            clsid,
            []()->UObjectPtr {
                return MakeStrongPtr<T>();
            }
        });
    }

public:
    TypesStaticRegistrarRedux() {
        // register all classes !!!
#define REGISTER_UCLASS(cls, type)  this->RegisterClass<type>(CRC32(cls))

        REGISTER_UCLASS("EFFECT",       UObjectEffectRedux);
        REGISTER_UCLASS("STATICPROP",   UObjectStatic);
        REGISTER_UCLASS("O_ENTITY",     UEntityRedux);

#undef REGISTER_UCLASS
    }

    MyDict<uint32_t, std::function<UObjectPtr()>>   registeredClasses;
};

static TypesStaticRegistrarRedux    sUTypes;



static void LogUnknownClassId(const uint32_t classId) {
    const auto& s = MetroTypedStrings::Get().GetString(classId);
    if (!s.Valid()) {
        LogPrintF(LogLevel::Error, "UObject, unknown classId [%08X]", classId);
    } else {
        LogPrintF(LogLevel::Warning, "UObject, unhandled classId [%s] [%08X]", s.str.c_str(), classId);
    }
}



ReduxEntityFactory::ReduxEntityFactory() {}
ReduxEntityFactory::~ReduxEntityFactory() {}

UObjectPtr ReduxEntityFactory::CreateUObject(const UObjectInitData& initData) {
    UObjectPtr object;

    auto it = sUTypes.registeredClasses.find(initData.cls);
    if (it != sUTypes.registeredClasses.end()) {
        object = it->second();
        object->initData = initData;
        object->cls = MetroTypedStrings::Get().GetString(initData.cls).str;
    } else {
        LogUnknownClassId(initData.cls);
        object = MakeStrongPtr<UnknownObject>();
    }

    return std::move(object);
}
