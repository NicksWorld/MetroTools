#include "LevelGeoNode.h"
#include "engine/LevelGeo.h"

namespace u4a {

LevelGeoNode::LevelGeoNode()
    : SceneNode()
    , mLevelGeo(nullptr)
{
    mType = NodeType::LevelGeo;
}
LevelGeoNode::~LevelGeoNode() {
}

void LevelGeoNode::SetLevelGeo(LevelGeo* geo) {
    mLevelGeo = geo;
}

LevelGeo* LevelGeoNode::GetLevelGeo() const {
    return mLevelGeo;
}

} // namespace u4a

