#pragma once
#include "SceneNode.h"

namespace u4a {

class DebugGeo;

class DebugGeoNode final : public SceneNode {
public:
    DebugGeoNode();
    ~DebugGeoNode();

    // ModelNode impl
    void            SetDebugGeo(DebugGeo* geo);
    DebugGeo*       GetDebugGeo() const;

private:
    DebugGeo*       mDebugGeo;
};

} // namespace u4a
