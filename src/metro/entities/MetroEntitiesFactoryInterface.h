#pragma once
#include "MetroUObject.h"

struct MetroEntitiesFactoryInterface {
protected:
    MetroEntitiesFactoryInterface() = default;
    virtual ~MetroEntitiesFactoryInterface() = default;

public:
    virtual UObjectPtr  CreateUObject(const UObjectInitData& initData) = 0;
};

using MetroEntitiesFactoryRPtr = RefPtr<MetroEntitiesFactoryInterface>;

MetroEntitiesFactoryRPtr GetEntitiesFactoryForVersion(const size_t version);
