#include "DebugGeoNode.h"
#include "engine/DebugGeo.h"

namespace u4a {

DebugGeoNode::DebugGeoNode()
    : mDebugGeo(nullptr)
{
    mType = NodeType::DebugGeo;
}
DebugGeoNode::~DebugGeoNode() {
}

// ModelNode impl
void DebugGeoNode::SetDebugGeo(DebugGeo* geo) {
    mDebugGeo = geo;
}

DebugGeo* DebugGeoNode::GetDebugGeo() const {
    return mDebugGeo;
}

} // namespace u4a
