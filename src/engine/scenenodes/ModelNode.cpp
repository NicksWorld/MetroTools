#include "ModelNode.h"
#include "engine/Model.h"
#include "engine/Animator.h"

namespace u4a {

ModelNode::ModelNode()
    : SceneNode()
    , mModel(nullptr)
    , mAnimator(nullptr)
    , mBSphere{vec3(0.0f), 0.0f}
{
    mType = NodeType::Model;
}
ModelNode::~ModelNode() {
    MySafeDelete(mAnimator);
}


// SceneNode impl
void ModelNode::Update(const float dt, const bool newTransform) {
    if (mAnimator) {
        mAnimator->Update(dt);
    }
    if (mModel && newTransform) {
        const mat4& transform = this->GetTransform();

        const BSphere& bsphere = mModel->GetBSphere();
        mBSphere.center = transform * vec4(bsphere.center, 1.0f);
        mBSphere.radius = bsphere.radius * MaxComponent(this->GetScale());

        const AABBox& aabb = mModel->GetBBox();
        mAABB.Reset();

        vec3 points[8] = {
            aabb.minimum,
            vec3(aabb.minimum.x, aabb.minimum.y, aabb.maximum.z),
            vec3(aabb.minimum.x, aabb.maximum.y, aabb.minimum.z),
            vec3(aabb.minimum.x, aabb.maximum.y, aabb.maximum.z),
            vec3(aabb.maximum.x, aabb.minimum.y, aabb.minimum.z),
            vec3(aabb.maximum.x, aabb.minimum.y, aabb.maximum.z),
            vec3(aabb.maximum.x, aabb.maximum.y, aabb.minimum.z),
            aabb.maximum,
        };
        for (const vec3& pt : points) {
            mAABB.Absorb(transform * vec4(pt, 1.0f));
        }
    }
}


void ModelNode::SetModel(Model* model, const bool needAnimator) {
    mModel = model;

    if (mModel && mModel->GetType() == Model::Type::Skinned && needAnimator) {
        MySafeDelete(mAnimator);
        mAnimator = new Animator();
        mAnimator->SetSkeleton(mModel->GetSkeleton());
    }

    if (mModel) {
        mBSphere = mModel->GetBSphere();
    }
}

Model* ModelNode::GetModel() const {
    return mModel;
}

Animator* ModelNode::GetAnimator() const {
    return mAnimator;
}

const BSphere& ModelNode::GetBSphere() const {
    return mBSphere;
}

const AABBox& ModelNode::GetAABB() const {
    return mAABB;
}

} // namespace u4a

