#include "Animator.h"

#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"

namespace u4a {

Animator::Animator()
    : mSkeleton{}
    , mMotion{}
    , mState(AnimState::Stopped)
    , mTimer(0.0f)
    , mAnimTime(0.0f)
{
}
Animator::~Animator() {
}


void Animator::SetSkeleton(const RefPtr<MetroSkeleton>& skeleton) {
    mSkeleton = skeleton;

    if (mSkeleton) {
        mAnimResult.resize(mSkeleton->GetNumBones(), MatIdentity);

        //#NOTE_SK: this should be in skeleton!!!
        this->FlattenBones();
    }
}

void Animator::SetMotion(const RefPtr<MetroMotion>& motion) {
    mMotion = motion;

    if (mMotion) {
        mAnimTime = mMotion->GetMotionTimeInSeconds();
        mTimer = 0.0f;
    }
}

void Animator::Update(const float dt) {
    if (mSkeleton && mMotion && AnimState::Playing == mState) {
        const size_t key = scast<size_t>(std::floorf((mTimer / mAnimTime) * mMotion->GetNumFrames()));

        const size_t numBones = mSkeleton->GetNumBones();
        for (size_t i = 0; i < numBones; ++i) {
            const AnimBone& b = mFlattenedBones[i];

            mat4& m = mAnimResult[b.id];

            if (mMotion->IsBoneAnimated(b.id)) {
                quat q = mMotion->GetBoneRotation(b.id, key);
                vec3 t = mMotion->GetBonePosition(b.id, key);

                m = MatFromQuat(q);
                m[3] = vec4(t, 1.0f);
            } else {
                m = mSkeleton->GetBoneTransform(b.id);
            }

            if (b.pid != kInvalidValue) {
                m = mAnimResult[b.pid] * m;
            }
        }

        for (size_t i = 0; i < numBones; ++i) {
            mat4& m = mAnimResult[i];
            m = m * mSkeleton->GetBoneFullTransformInv(i);
        }

        mTimer += dt;
        if (mTimer > mAnimTime) {   // loop
            mTimer -= mAnimTime;
        }
    }
}

void Animator::Stop() {
    mState = AnimState::Stopped;
}

void Animator::Play() {
    if (AnimState::Stopped == mState) {
        mTimer = 0.0f;
    }

    mState = AnimState::Playing;
}

void Animator::Pause() {
    mState = AnimState::Paused;
}

const Animator::AnimState Animator::GetAnimState() const {
    return mState;
}

const Mat4Array& Animator::GetAnimResult() const {
    return mAnimResult;
}


struct HierarchyBone {
    Animator::AnimBone      srcBone;
    MyArray<HierarchyBone*> children;
};

static void FlattenHierarchyToArray(Animator::AnimBone*& arr, const HierarchyBone* hb) {
    // parents go first
    for (const HierarchyBone* next : hb->children) {
        *arr = next->srcBone;
        ++arr;
    }

    // children go next
    for (const HierarchyBone* next : hb->children) {
        FlattenHierarchyToArray(arr, next);
    }
}

void Animator::FlattenBones() {
    const size_t numBones = mSkeleton->GetNumBones();
    if (numBones) {
        MyArray<HierarchyBone> hierarchy(numBones);

        mFlattenedBones.resize(numBones);

        size_t rootBoneIdx = 0;
        for (size_t i = 0; i < numBones; ++i) {
            AnimBone& b = mFlattenedBones[i];
            b.id = i;
            b.pid = mSkeleton->GetBoneParentIdx(i);

            hierarchy[b.id].srcBone = b;
            if (b.pid != kInvalidValue) {
                hierarchy[b.pid].children.push_back(&hierarchy[b.id]);
            } else {
                rootBoneIdx = i;
            }
        }

        // now we flatten our hierarchy so that parent bones are always come before their children
        mFlattenedBones[0] = mFlattenedBones[rootBoneIdx];
        if (numBones > 1) {
            AnimBone* arr = &mFlattenedBones[1];
            FlattenHierarchyToArray(arr, &hierarchy[rootBoneIdx]);
        }
    }
}

} // namespace u4a
