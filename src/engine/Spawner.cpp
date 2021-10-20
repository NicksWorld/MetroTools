#include "Spawner.h"

#include "engine/Scene.h"
#include "engine/scenenodes/ModelNode.h"
#include "engine/scenenodes/LevelGeoNode.h"
#include "engine/Model.h"
#include "engine/ResourcesManager.h"

#include "metro/MetroLevel.h"

namespace u4a {

SceneNode* Spawner::SpawnModel(Scene& scene, const CharString& modelName, const vec3& pos, const bool needAnimator, SceneNode* parent) {
    SceneNode* result = nullptr;

    Model* model = ResourcesManager::Get().GetModel(modelName, needAnimator);
    if (model) {
        ModelNode* node = new ModelNode();
        node->SetModel(model, needAnimator);

        node->SetPosition(pos);

        //!!hack
        if (reinterpret_cast<MyHandle>(parent) != kInvalidHandle) {
            if (!parent) {
                parent = scene.GetRootNode();
            }
            parent->AttachChild(node);
        }

        result = node;
    }

    return result;
}

SceneNode* Spawner::SpawnModel(Scene& scene, MetroModelBase* srcModel, const vec3& pos, const bool needAnimator, SceneNode* parent) {
    SceneNode* result = nullptr;

    Model* model = ResourcesManager::Get().ConstructModel(srcModel);
    if (model) {
        ModelNode* node = new ModelNode();
        node->SetModel(model, needAnimator);

        if (!parent) {
            parent = scene.GetRootNode();
        }

        node->SetPosition(pos);
        parent->AttachChild(node);

        result = node;
    }

    return result;
}

SceneNode* Spawner::SpawnLevelGeo(Scene& scene, MetroLevel* srcLevel, const vec3& pos, SceneNode* parent) {
    SceneNode* result = nullptr;

    LevelGeo* geo = ResourcesManager::Get().ConstructLevelGeo(srcLevel);
    if (geo) {
        LevelGeoNode* node = new LevelGeoNode();
        node->SetLevelGeo(geo);

        if (!parent) {
            parent = scene.GetRootNode();
        }

        node->SetPosition(pos);
        parent->AttachChild(node);

        // entities
        const size_t numEntities = srcLevel->GetNumEntities();
        if (numEntities) {
            SceneNode* hackParent = reinterpret_cast<SceneNode*>(kInvalidHandle);

            MyArray<SceneNode*> entNodes; entNodes.reserve(numEntities);
            MyArray<size_t> entNodesIds; entNodesIds.reserve(numEntities);
            MyArray<size_t> entIdx; entIdx.reserve(numEntities);
            for (size_t i = 0; i < numEntities; ++i) {
                const CharString& visual = srcLevel->GetEntityVisual(i);
                if (!visual.empty()) {
                    const CharString& fullModelName = visual;
                    //const size_t atPos = fullModelName.find('@');
                    //if (atPos != CharString::npos) {
                    //    fullModelName = fullModelName.substr(0, atPos);
                    //}

                    SceneNode* child = Spawner::SpawnModel(scene, fullModelName, vec3(0.0f), false, hackParent);
                    if (child) {
                        entNodes.push_back(child);
                        entNodesIds.push_back(srcLevel->GetEntityID(i));
                        entIdx.push_back(i);

                        child->SetUserData(i);

                        scast<ModelNode*>(child)->GetModel()->SetSource(visual);
                    }
                }
            }

            auto idsBegin = entNodesIds.begin();
            auto idsEnd = entNodesIds.end();
            for (size_t i = 0, end = entNodes.size(); i < end; ++i) {
                SceneNode* child = entNodes[i];

                const size_t idx = entIdx[i];
                const size_t id = srcLevel->GetEntityID(idx);
                const size_t pid = srcLevel->GetEntityParentID(idx);

                SceneNode* attachTo = node;

                mat4 pose = MatFromPose(srcLevel->GetEntityTransform(idx));

                //const CharString& boneName = srcLevel->GetEntityAttachBone(idx);
                //if (pid != 0xFFFF && !boneName.empty()) {
                //    auto it = std::lower_bound(idsBegin, idsEnd, pid);
                //    if (it != idsEnd) {
                //        const size_t j = std::distance(idsBegin, it);
                //        attachTo = entNodes[j];

                //        mat4 attPose = MatFromPose(srcLevel->GetEntityAttachTransform(idx));
                //        mat4 boneMat = scast<ModelNode*>(attachTo)->GetModel()->GetBoneTransform(boneName);
                //        pose = attPose * boneMat;
                //    }
                //}

                vec3 pos, scale;
                quat rot;
                MatDecompose(pose, pos, scale, rot);

                child->SetPosition(pos);
                child->SetScale(scale);
                child->SetRotation(rot);

                attachTo->AttachChild(child);
            }
        }

        result = node;
    }

    return result;
}

} // namespace u4a

