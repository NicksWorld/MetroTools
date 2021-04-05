#pragma once
#include "MetroUObject.h"
#include "MetroEntity.h"

class MetroEntityFactory {
    IMPL_SINGLETON(MetroEntityFactory)

private:
    MetroEntityFactory();

public:
    UObjectPtr  CreateUObject(const UObjectInitData& initData);

private:
    template <typename T>
    inline void RegisterClass(const uint32_t clsid) {
        mRegisteredClasses.insert({
            clsid,
            []()->UObjectPtr {
                return MakeStrongPtr<T>();
            }
        });
    }

private:
    MyDict<uint32_t, std::function<UObjectPtr()>>   mRegisteredClasses;
};
