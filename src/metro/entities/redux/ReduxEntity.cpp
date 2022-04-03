#include "ReduxEntity.h"
#include "metro/reflection/MetroReflection.h"


// UEntities

void UEntityRedux::Serialize(MetroReflectionStream& s) {
    //e.ReadFP32("health");
    //e.ReadFP32("stamina");
    //e.ReadU32("dying_mask");
    s.SkipBytes(12);
    //
    METRO_SERIALIZE_BASE_CLASS(s);
    //
}
