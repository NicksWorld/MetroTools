#include "exodus/ExodusEntityFactory.h"
#include "redux/ReduxEntityFactory.h"

MetroEntitiesFactoryRPtr GetEntitiesFactoryForVersion(const size_t version) {
    static RefPtr<ExodusEntityFactory>  sExodusFactory;
    static RefPtr<ReduxEntityFactory>   sReduxFactory;

    if (version >= UObject::kVersionArktika1) {
        if (!sExodusFactory) {
            sExodusFactory = MakeRefPtr<ExodusEntityFactory>();
        }

        return sExodusFactory;
    } else {
        if (!sReduxFactory) {
            sReduxFactory = MakeRefPtr<ReduxEntityFactory>();
        }

        return sReduxFactory;
    }
}
