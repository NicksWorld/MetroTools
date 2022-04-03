#pragma once
#include "../MetroEntitiesFactoryInterface.h"

class ExodusEntityFactory : public MetroEntitiesFactoryInterface {
public:
    ExodusEntityFactory();
    virtual ~ExodusEntityFactory();

    virtual UObjectPtr  CreateUObject(const UObjectInitData& initData) override;
};
