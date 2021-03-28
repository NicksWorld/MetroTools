#include "SceneNode.h"

namespace u4a {

SceneNode::SceneNode()
    : mType(NodeType::Dummy)
    , mUserData(0)
    , mPosition(0.0f)
    , mRotation(1.0f, 0.0f, 0.0f, 0.0f)
    , mScale(1.0f)
    , mLocalTransform(1.0f)
    , mTransform(1.0f)
    , mTransformDirty(true)
    , mParent(nullptr)
{
}
SceneNode::~SceneNode() {
    this->DeleteChildren();
}


void SceneNode::DeleteChildren() {
    for (auto child : mChildren) {
        child->DeleteChildren();
        MySafeDelete(child);
    }
    mChildren.clear();
}

SceneNode::NodeType SceneNode::GetType() const {
    return mType;
}

void SceneNode::SetName(const CharString& name) {
    mName = name;
}

const CharString& SceneNode::GetName() const {
    return mName;
}

void SceneNode::SetUserData(uintptr_t userData) {
    mUserData = userData;
}

uintptr_t SceneNode::GetUserData() const {
    return mUserData;
}

void SceneNode::SetPosition(const vec3& pos) {
    if (mPosition != pos) {
        mPosition = pos;
        mTransformDirty = true;
    }
}

const vec3& SceneNode::GetPosition() const {
    return mPosition;
}

void SceneNode::SetRotation(const quat& rotation) {
    if (mRotation != rotation) {
        mRotation = rotation;
        mTransformDirty = true;
    }
}

const quat& SceneNode::GetRotation() const {
    return mRotation;
}

void SceneNode::SetScale(const vec3& scale) {
    if (mScale != scale) {
        mScale = scale;
        mTransformDirty = true;
    }
}

const vec3& SceneNode::GetScale() const {
    return mScale;
}

bool SceneNode::UpdateTransform(const mat4& parentTransform, const bool parentIsDirty) {
    const bool updateTransform = mTransformDirty || parentIsDirty;

    if (mTransformDirty) {
        mLocalTransform = MatFromQuat(mRotation);
        mLocalTransform[0] *= mScale.x;
        mLocalTransform[1] *= mScale.y;
        mLocalTransform[2] *= mScale.z;
        mLocalTransform[3] = vec4(mPosition, 1.0f);

        mTransformDirty = false;
    }

    if (updateTransform) {
        mTransform = parentTransform * mLocalTransform;
    }

    return updateTransform;
}

const mat4& SceneNode::GetLocalTransform() const {
    return mLocalTransform;
}

const mat4& SceneNode::GetTransform() const {
    return mTransform;
}

void SceneNode::AttachChild(SceneNode* node) {
    mChildren.push_back(node);
    node->mParent = this;
}

void SceneNode::UnattachChild(SceneNode* node) {
    auto it = std::find(mChildren.begin(), mChildren.end(), node);
    if (it != mChildren.end()) {
        node->mParent = nullptr;
        mChildren.erase(it);
    }
}

SceneNode* SceneNode::GetParent() const {
    return mParent;
}

size_t SceneNode::GetNumChildren() const {
    return mChildren.size();
}

SceneNode* SceneNode::GetChild(const size_t idx) const {
    return mChildren[idx];
}

} // namespace u4a
