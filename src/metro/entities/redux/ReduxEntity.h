#pragma once
#include "ReduxUObject.h"

// UEntities

struct UEntityRedux : public UObjectEffectRedux {
    INHERITED_CLASS(UObjectEffectRedux);

    void Serialize(MetroReflectionStream& s) override;
};
