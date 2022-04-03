#pragma once
#include "../MetroEntitiesFactoryInterface.h"

class ReduxEntityFactory : public MetroEntitiesFactoryInterface {
public:
    ReduxEntityFactory();
    virtual ~ReduxEntityFactory();

    virtual UObjectPtr  CreateUObject(const UObjectInitData& initData) override;
};
