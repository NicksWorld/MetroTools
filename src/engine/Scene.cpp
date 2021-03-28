#include "Scene.h"

namespace u4a {

Scene::Scene()
    : mRootNode(nullptr)
    , mCamera(nullptr)
    , mFrameDT(0.0f)
{
}

Scene::~Scene() {
    this->Shutdown();
}


bool Scene::Initialize() {
    mRootNode = new SceneNode();
    return true;
}

void Scene::Shutdown() {
    MySafeDelete(mRootNode);
    mCamera = nullptr;
}

void Scene::Clear() {
    mRootNode->DeleteChildren();
}

void Scene::SetCamera(Camera* camera) {
    mCamera = camera;
}

Camera* Scene::GetCamera() const {
    return mCamera;
}

void Scene::Update(const float dt) {
    mFrameDT = dt;
    this->RecursiveUpdateNode(mRootNode, false);
}

SceneNode* Scene::GetRootNode() {
    return mRootNode;
}

void Scene::CollectNodesByType(const SceneNode::NodeType type, SceneNodesArray& resultArray) {
    this->RecursiveCollectNodes(type, resultArray, mRootNode);
}


void Scene::RecursiveUpdateNode(SceneNode* node, const bool parentTransformDirty) {
    const mat4& parentTransform = node->GetParent() ? node->GetParent()->GetTransform() : MatIdentity;
    const bool transformUpdated = node->UpdateTransform(parentTransform, parentTransformDirty);
    node->Update(mFrameDT, transformUpdated);

    const mat4& transform = node->GetTransform();
    for (size_t i = 0, end = node->GetNumChildren(); i < end; ++i) {
        this->RecursiveUpdateNode(node->GetChild(i), transformUpdated);
    }
}

void Scene::RecursiveCollectNodes(const SceneNode::NodeType type, SceneNodesArray& resultArray, SceneNode* node) {
    if (type == node->GetType()) {
        resultArray.push_back(node);
    }

    for (size_t i = 0, end = node->GetNumChildren(); i < end; ++i) {
        this->RecursiveCollectNodes(type, resultArray, node->GetChild(i));
    }
}

} // namespace u4a
