#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroModel;
class MetroModelBase;
class MetroLevel;

namespace u4a {

class Scene;
class SceneNode;

class DebugGeo;

struct Spawner {
    static SceneNode*   SpawnModel(Scene& scene, const CharString& modelName, const vec3& pos, const bool needAnimator, SceneNode* parent = nullptr);
    static SceneNode*   SpawnModel(Scene& scene, MetroModelBase* srcModel, const vec3& pos, const bool needAnimator, SceneNode* parent = nullptr);

    static SceneNode*   SpawnLevelGeo(Scene& scene, MetroLevel* srcLevel, const vec3& pos, SceneNode* parent = nullptr);

    static SceneNode*   SpawnDebugGeoNode(Scene& scene, DebugGeo* debugGeo, const vec3& pos, SceneNode* parent = nullptr);
};

} // namespace u4a
