#pragma once
#include "SceneNode.h"

namespace u4a {

class Model;
class Animator;

class ModelNode final : public SceneNode {
public:
    ModelNode();
    ~ModelNode();

    // SceneNode impl
    void            Update(const float dt, const bool newTransform) override;

    // ModelNode impl
    void            SetModel(Model* model, const bool needAnimator);
    Model*          GetModel() const;
    Animator*       GetAnimator() const;

    const BSphere&  GetBSphere() const;
    const AABBox&   GetAABB() const;

private:
    Model*      mModel;
    Animator*   mAnimator;
    BSphere     mBSphere;
    AABBox      mAABB;
};

} // namespace u4a
