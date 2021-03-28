#pragma once
#include "MetroUObject.h"
#include "MetroEntity.h"

struct MetroEntityFactory {
    static UObjectPtr   CreateUObject(const UObjectInitData& initData);
};
