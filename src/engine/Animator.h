#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroSkeleton;
class MetroMotion;

namespace u4a {

class Animator {
public:
    struct AnimBone {
        size_t  id;
        size_t  pid;
    };

    enum class AnimState : size_t {
        Stopped,
        Playing,
        Paused
    };

public:
    Animator();
    ~Animator();

    void                    SetSkeleton(const RefPtr<MetroSkeleton>& skeleton);
    void                    SetMotion(const RefPtr<MetroMotion>& motion);

    void                    Update(const float dt);
    void                    Stop();
    void                    Play();
    void                    Pause();
    const AnimState         GetAnimState() const;

    const Mat4Array&        GetAnimResult() const;

private:
    void                    FlattenBones();

private:
    RefPtr<MetroSkeleton>   mSkeleton;
    RefPtr<MetroMotion>     mMotion;
    AnimState               mState;
    float                   mTimer;
    float                   mAnimTime;
    MyArray<AnimBone>       mFlattenedBones;
    Mat4Array               mAnimResult;
};

} // namespace u4a
