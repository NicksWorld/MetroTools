#pragma once
#include "mycommon.h"
#include "mymath.h"

namespace u4a {

class SceneNode;
using SceneNodesArray = MyArray<SceneNode*>;

class SceneNode {
public:
    enum class NodeType : size_t {
        Dummy,
        Model,
        LevelGeo,
        DebugGeo
    };

public:
    SceneNode();
    virtual ~SceneNode();

    void                DeleteChildren();

    NodeType            GetType() const;

    void                SetName(const CharString& name);
    const CharString&   GetName() const;

    void                SetUserData(uintptr_t userData);
    uintptr_t           GetUserData() const;

    void                SetPosition(const vec3& pos);
    const vec3&         GetPosition() const;
    void                SetRotation(const quat& rotation);
    const quat&         GetRotation() const;
    void                SetScale(const vec3& scale);
    const vec3&         GetScale() const;

    bool                UpdateTransform(const mat4& parentTransform, const bool parentIsDirty);
    virtual void        Update(const float /*dt*/, const bool /*newTransform*/) { }

    const mat4&         GetLocalTransform() const;
    const mat4&         GetTransform() const;

    void                AttachChild(SceneNode* node);
    void                UnattachChild(SceneNode* node);

    SceneNode*          GetParent() const;

    size_t              GetNumChildren() const;
    SceneNode*          GetChild(const size_t idx) const;

protected:
    NodeType            mType;
    CharString          mName;
    uintptr_t           mUserData;
    vec3                mPosition;
    quat                mRotation;
    vec3                mScale;
    mat4                mLocalTransform;
    mat4                mTransform;
    bool                mTransformDirty;
    SceneNode*          mParent;
    SceneNodesArray     mChildren;
};

} // namespace u4a
