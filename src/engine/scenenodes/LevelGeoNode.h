#pragma once
#include "SceneNode.h"

namespace u4a {

class LevelGeo;

class LevelGeoNode final : public SceneNode {
public:
    LevelGeoNode();
    ~LevelGeoNode();

    void        SetLevelGeo(LevelGeo* geo);
    LevelGeo*   GetLevelGeo() const;

private:
    LevelGeo*   mLevelGeo;
};

} // namespace u4a
