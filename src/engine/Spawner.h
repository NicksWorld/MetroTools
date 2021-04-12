#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroModel;
class MetroModelBase;
class MetroLevel;

namespace u4a {

class Scene;
class SceneNode;

struct Spawner {
    static SceneNode*   SpawnModel(Scene& scene, const CharString& modelName, const vec3& pos, const bool needAnimator, SceneNode* parent = nullptr);
    static SceneNode*   SpawnModelNew(Scene& scene, MetroModelBase* srcModel, const vec3& pos, const bool needAnimator, SceneNode* parent = nullptr);

    static SceneNode*   SpawnLevelGeo(Scene& scene, MetroLevel* srcLevel, const vec3& pos, SceneNode* parent = nullptr);
};

} // namespace u4a
