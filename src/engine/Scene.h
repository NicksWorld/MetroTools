#pragma once
#include "mycommon.h"
#include "mymath.h"

#include "scenenodes/SceneNode.h"

namespace u4a {

class Camera;

class Scene {
public:
    Scene();
    ~Scene();

    bool                Initialize();
    void                Shutdown();
    void                Clear();

    void                SetCamera(Camera* camera);
    Camera*             GetCamera() const;

    void                Update(const float dt);

    SceneNode*          GetRootNode();

    void                CollectNodesByType(const SceneNode::NodeType type, SceneNodesArray& resultArray);

private:
    void                RecursiveUpdateNode(SceneNode* node, const bool parentTransformDirty);
    void                RecursiveCollectNodes(const SceneNode::NodeType type, SceneNodesArray& resultArray, SceneNode* node);

private:
    SceneNode*          mRootNode;
    Camera*             mCamera;
    float               mFrameDT;
};

} // namespace u4a
