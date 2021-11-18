#pragma once
#include "mycommon.h"
#include "mymath.h"
#include "metro/MetroTypes.h"

class MetroPhysicsCForm;
class MetroPhysicsCMesh;
class MetroPhysicsCollection;
class MetroPhysicsTrimesh;
class MetroPhysicsConvex;
class MetroPhysicsShape;
class MetroPhysicsModel;
class MetroPhysicsModelInstance;
class MetroPhysicsScene;
class MetroPhysicsMaterial;
class MetroPhysicsActor;
class MetroPhysicsJoint;
class MetroPhysicsSpringAndDamperEffector;

class NxuBinaryStream;

RefPtr<MetroPhysicsCollection>  MetroPhysicsLoadCollectionFromStream(MemStream srcStream);
RefPtr<MetroPhysicsCForm>       MetroPhysicsLoadCFormFromStream(MemStream srcStream, const bool isLevelGeo);
void                            MetroPhysicsWriteCFormToStream(const RefPtr<MetroPhysicsCForm>& cform, MemWriteStream& dstStream, const MetroGameVersion gameVersion, const bool isLevelGeo);

struct MetroPhysicsSpring {
    float   spring;
    float   damper;
    float   targetValue;

    static void Read(NxuBinaryStream* stream, MetroPhysicsSpring& spring);
};

struct MetroPhysicsLimit {
    float   hardness;
    float   restitution;
    float   value;

    static void Read(NxuBinaryStream* stream, MetroPhysicsLimit& limit);
};

struct MetroPhysicsMotor {
    bool    freeSpin;
    float   maxForce;
    float   velTarget;

    static void Read(NxuBinaryStream* stream, MetroPhysicsMotor& motor);
};

struct MetroPhysicsPairFlag {
    uint32_t    flag;
    uint32_t    actor0Index;
    uint32_t    shape0Index;
    uint32_t    actor1Index;
    uint32_t    shape1Index;

    static void Read(NxuBinaryStream* stream, MetroPhysicsPairFlag& pairFlag);
};


class MetroPhysicsCForm {
public:
    MetroPhysicsCForm();
    ~MetroPhysicsCForm();

    static uint32_t             GameVersionToCFormVersion(const MetroGameVersion gameVersion);

    bool                        Read(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo);
    void                        Write(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) const;
    size_t                      GetNumCMeshes() const;
    RefPtr<MetroPhysicsCMesh>   GetCMesh(const size_t idx) const;
    void                        AddCMesh(const RefPtr<MetroPhysicsCMesh>& mesh);

private:
    MyArray<RefPtr<MetroPhysicsCMesh>>  mMeshes;
};

class MetroPhysicsCMesh {
public:
    MetroPhysicsCMesh();
    ~MetroPhysicsCMesh();

    bool                Read(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo);
    void                Write(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) const;

    uint16_t            GetSector() const;
    void                SetSector(const uint16_t sector);
    uint16_t            GetCollisionGroup() const;
    void                SetCollisionGroup(const uint16_t colGroup);
    bool                IsPrimitive() const;
    void                SetIsPrimitive(const bool b);
    bool                IsRaycast() const;
    void                SetIsRaycast(const bool b);

    const CharString&   GetShader() const;
    void                SetShader(const CharString& shader);
    const CharString&   GetTexture() const;
    void                SetTexture(const CharString& texture);
    const CharString&   GetMaterialName1() const;
    void                SetMaterialName1(const CharString& material);
    const CharString&   GetMaterialName2() const;
    void                SetMaterialName2(const CharString& material);

    BytesArray&         GetCookedData();
    void                SetCookedData(const void* data, const size_t dataSize);

private:
    bool                ReadMaterial(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo);
    void                WriteMaterial(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) const;

private:
    uint16_t            mDummy;
    uint16_t            mSector;
    uint16_t            mCollisionGroup;
    bool                mIsDummy;
    bool                mIsPrimitive;
    bool                mIsRaycast;
    CharString          mShader;
    CharString          mTexList;
    CharString          mGameMaterial;
    CharString          mMaterialName1;
    CharString          mMaterialName2;
    BytesArray          mCookedData;
};


class MetroPhysicsCollection {
public:
    struct NxuParameterData {
        uint32_t    param;
        float       value;
    };

public:
    MetroPhysicsCollection();
    virtual ~MetroPhysicsCollection();

    virtual bool    Load(NxuBinaryStream* stream);

private:
    MyArray<NxuParameterData>       mParams;
    MyArray<MetroPhysicsConvex*>    mConvexes;
    MyArray<MetroPhysicsTrimesh*>   mTrimeshes;
    MyArray<MetroPhysicsScene*>     mScenes;
    MyArray<MetroPhysicsModel*>     mModels;
    StrongPtr<MetroPhysicsModel>    mDefaultModel;
    StrongPtr<MetroPhysicsActor>    mDefaultActor;
};

class MetroPhysicsModel : MetroPhysicsCollection {
    INHERITED_CLASS(MetroPhysicsCollection)
public:
    MetroPhysicsModel();
    virtual ~MetroPhysicsModel();

    virtual bool    Load(NxuBinaryStream* stream) override;

    void            AddMaterial(MetroPhysicsMaterial* material);
    void            AddActor(MetroPhysicsActor* actor);
    void            AddJoint(MetroPhysicsJoint* joint);
    void            AddPairFlag(const MetroPhysicsPairFlag& pairFlag);
    void            AddEffector(MetroPhysicsSpringAndDamperEffector* effector);

private:
    CharString                      mName;
    MyArray<MetroPhysicsMaterial*>  mMaterials;
    MyArray<MetroPhysicsActor*>     mActors;
    MyArray<MetroPhysicsJoint*>     mJoints;
    MyArray<MetroPhysicsPairFlag>   mPairFlags;
    MyArray<MetroPhysicsSpringAndDamperEffector*>   mEffectors;
    MyArray<uint32_t>               mCollisionGroupA;
    MyArray<uint32_t>               mCollisionGroupB;
    MyArray<MetroPhysicsModelInstance*> mModels;
};

class MetroPhysicsModelInstance {
public:
    MetroPhysicsModelInstance();
    ~MetroPhysicsModelInstance();

    bool    Load(NxuBinaryStream* stream);

private:
    mat4        mPose;
    uint32_t    mModelIdx;
};

class MetroPhysicsSpringAndDamperEffector {
public:
    MetroPhysicsSpringAndDamperEffector();
    ~MetroPhysicsSpringAndDamperEffector();

    bool        Load(NxuBinaryStream* stream);

private:
    uint32_t    mRefAttachActorDesc;
    uint32_t    mAttachActorDesc;
    vec3        mPos1;
    vec3        mPos2;
    float       mSpringDistCompressSaturate;
    float       mSpringDistRelaxed;
    float       mSpringDistStretchSaturate;
    float       mSpringMaxCompressForce;
    float       mSpringMaxStretchForce;
    float       mDamperVelCompressSaturate;
    float       mDamperVelStretchSaturate;
    float       mDamperMaxCompressForce;
    float       mDamperMaxStretchForce;
};

class MetroPhysicsConvex {
public:
    MetroPhysicsConvex();
    ~MetroPhysicsConvex();

    bool    Load(NxuBinaryStream* stream);

    bool    AreIndices16Bit() const;

private:
    uint32_t    mFlags;
    uint32_t    mNumVertices;
    uint32_t    mVertexSize;
    uint32_t    mNumTriangles;
    uint32_t    mTriangleSize;
    BytesArray  mVerticesData;
    BytesArray  mTrianglesData;
    BytesArray  mCookedData;
};

class MetroPhysicsTrimesh {
public:
    MetroPhysicsTrimesh();
    ~MetroPhysicsTrimesh();

    bool    Load(NxuBinaryStream* stream);

private:
    bool        mHasMaterialIndicies;
    uint32_t    mHeightFieldVerticalAxis;   // enum
    float       mHeightFieldVerticalExtent;
    float       mConvexEdgeThreshold;
    uint32_t    mFlags;
    uint32_t    mNumVertices;
    uint32_t    mVertexSize;    // 12 -> float, 24 -> double
    uint32_t    mNumTriangles;
    uint32_t    mTriangleSize;  // 3 -> byte, 6 -> word, 12 -> dword
    BytesArray  mVerticesData;
    BytesArray  mTrianglesData;
    BytesArray  mCookedData;
    BytesArray  mPmap;
};

class MetroPhysicsMaterial {
public:
    MetroPhysicsMaterial();
    ~MetroPhysicsMaterial();

    bool        Load(NxuBinaryStream* stream);

private:
    uint16_t    mIndex;
    float       mDynamicFriction;
    float       mStaticFriction;
    float       mRestitution;
    float       mDynamicFrictionV;
    float       mStaticFrictionV;
    uint32_t    mFrictionCombineMode;
    uint32_t    mRestitutionCombineMode;
    vec3        mDirOfAnisotropy;
    uint32_t    mFlags;
    bool        mHasSpring;
    float       mSpringSpring;
    float       mSpringDamper;
    float       mSpringTargetValue;
};

class MetroPhysicsActor {
public:
    MetroPhysicsActor();
    ~MetroPhysicsActor();

    bool        Load(NxuBinaryStream* stream);

    void        AddShape(MetroPhysicsShape* shape);

private:
    mat4        mGlobalPose;
    bool        mHasBody;
    // body
    mat4        mBodyMassLocalPose;
    vec3        mBodyMassSpaceInertia;
    float       mBodyMass;
    vec3        mBodyLinearVelocity;
    vec3        mBodyAngularVelocity;
    float       mBodyWakeUpCounter;
    float       mBodyLinearDamping;
    float       mBodyAngularDamping;
    float       mBodyMaxAngularVelocity;
    float       mBodyCCDMotionThreshold;
    uint32_t    mBodyFlags;
    float       mBodySleepLinearVelocity;
    float       mBodySleepAngularVelocity;
    uint32_t    mBodySolverIterationCount;
    //
    float       mDensity;
    uint32_t    mGroup;
    uint32_t    mFlags;
    uint32_t    mUserData;
    CharString  mName;
    MyArray<MetroPhysicsShape*> mShapes;
};

class MetroPhysicsShape {
public:
    enum class ShapeType {
        Plane       = 0,
        Sphere      = 1,
        Box         = 2,
        Capsule     = 3,
        Wheel       = 4,    // 2033 only ???
        Convex      = 5,
        TriMesh     = 6,
        Heightfield = 7     // 2033 only ???
    };

    static MetroPhysicsShape*  ReadShape(NxuBinaryStream* stream);

public:
    MetroPhysicsShape() {}
    virtual ~MetroPhysicsShape() {}

    virtual bool        Load(NxuBinaryStream* stream);
    virtual ShapeType   Type() const = 0;

protected:
    CharString  mName;
    mat4        mLocalPose;
    uint32_t    mGroup;
    uint32_t    mMaterialIndex;
    float       mMass;
    float       mDensity;
    float       mSkinWidth;
    uint32_t    mShapeFlags;
    uint32_t    mCCDSkeleton;
    Bitset128   mGroupsMask;    // not in 2033 !!!
};

class MetroPhysicsShapePlane final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    MetroPhysicsShapePlane();
    virtual ~MetroPhysicsShapePlane();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;
};

class MetroPhysicsShapeSphere final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    MetroPhysicsShapeSphere();
    virtual ~MetroPhysicsShapeSphere();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;

private:
    float   mRadius;
};

class MetroPhysicsShapeBox final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    MetroPhysicsShapeBox();
    virtual ~MetroPhysicsShapeBox();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;

private:
    vec3    mDimensions;
};

class MetroPhysicsShapeCapsule final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    MetroPhysicsShapeCapsule();
    virtual ~MetroPhysicsShapeCapsule();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;

private:
    float       mRadius;
    float       mHeight;
    uint32_t    mFlags;
};

class MetroPhysicsShapeWheel final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    struct TireFunction {
        float   mExtremumSlip;
        float   mExtremumValue;
        float   mAsymptoteSlip;
        float   mAsymptoteValue;
        float   mStiffnessFactor;
    };
public:
    MetroPhysicsShapeWheel();
    virtual ~MetroPhysicsShapeWheel();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;

private:
    void ReadTireFunction(NxuBinaryStream* stream, TireFunction& tireFunc);

private:
    float           mRadius;
    float           mSuspensionTravel;
    float           mSuspensionSpringSpring;
    float           mSuspensionSpringDamper;
    float           mSuspensionSpringTargetValue;
    TireFunction    mLongitudalTireForceFunction;
    TireFunction    mLateralTireForceFunction;
    float           mInverseWheelMass;
    uint32_t        mWheelFlags;
    float           mMotorTorque;
    float           mBrakeTorque;
    float           mSteerAngle;
};

class MetroPhysicsShapeConvex final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    MetroPhysicsShapeConvex();
    virtual ~MetroPhysicsShapeConvex();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;

private:
    uint32_t    mConvexMeshDesc;
    uint32_t    mMeshFlags;
    float       mScale;
};

class MetroPhysicsShapeTrimesh final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    MetroPhysicsShapeTrimesh();
    virtual ~MetroPhysicsShapeTrimesh();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;

private:
    uint32_t    mTrimeshMeshDesc;
    uint32_t    mMeshFlags;
    float       mScale;
};

class MetroPhysicsShapeHeightfield final : public MetroPhysicsShape {
    INHERITED_CLASS(MetroPhysicsShape)
public:
    MetroPhysicsShapeHeightfield();
    virtual ~MetroPhysicsShapeHeightfield();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual MetroPhysicsShape::ShapeType Type() const override;

private:
    uint32_t    mHeightFieldDesc;
    float       mHeightScale;
    float       mRowScale;
    float       mColumnScale;
    uint16_t    mMaterialIndexHighBits;
    uint16_t    mHoleMaterial;
    uint32_t    mMeshFlags;
};


class MetroPhysicsJoint {
public:
    enum class JointType {
        Prismatic,
        Revolute,
        Cylindrical,
        Spherical,
        PointOnLine,
        PointInPlane,
        Distance,
        Pulley,
        Fixed,
        D6
    };

    struct LimitPlane {
        vec3    normal;
        float   distance;
        vec3    worldLimitPt;
    };

    struct SoftLimit {
        float   value;
        float   restitution;
        float   spring;
        float   damping;
    };

    struct SoftPairLimit {
        SoftLimit   low;
        SoftLimit   high;
    };

    struct JointDrive {
        uint32_t    driveType;
        float       spring;
        float       damping;
        float       forceLimit;
    };

public:
    static MetroPhysicsJoint* ReadJoint(NxuBinaryStream* stream);

public:
    MetroPhysicsJoint();
    virtual ~MetroPhysicsJoint();

    virtual bool Load(NxuBinaryStream* stream);
    virtual JointType Type() const = 0;

protected:
    CharString          mName;
    uint32_t            mRefAttachActorDesc;
    uint32_t            mAttachActorDesc;
    vec3                mLocalNormal0;
    vec3                mLocalAxis0;
    vec3                mLocalAnchor0;
    vec3                mLocalNormal1;
    vec3                mLocalAxis1;
    vec3                mLocalAnchor1;
    float               mMaxForce;
    float               mMaxTorque;
    uint32_t            mJointFlags;
    uint32_t            mNumLimitPlanes;
    vec3                mPlaneLimitPoint;
    bool                mOnActor2;
    MyArray<LimitPlane> mLimitPlanes;
};

class MetroPhysicsJointPrismatic final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointPrismatic();
    virtual ~MetroPhysicsJointPrismatic();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;
};

class MetroPhysicsJointRevolute final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointRevolute();
    virtual ~MetroPhysicsJointRevolute();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;

private:
    MetroPhysicsLimit   mRevoluteLimitLow;
    MetroPhysicsLimit   mRevoluteLimitHigh;
    MetroPhysicsMotor   mRevoluteMotor;
    MetroPhysicsSpring  mRevoluteSpring;
    float               mRevoluteProjectionDistance;
    float               mRevoluteProjectionAngle;
    uint32_t            mRevoluteFlags;
    uint32_t            mRevoluteProjectionMode;
};

class MetroPhysicsJointCylindrical final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointCylindrical();
    virtual ~MetroPhysicsJointCylindrical();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;
};

class MetroPhysicsJointSpherical final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointSpherical();
    virtual ~MetroPhysicsJointSpherical();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;

private:
    MetroPhysicsSpring  mSphericalTwistSpring;
    MetroPhysicsSpring  mSphericalSwingSpring;
    MetroPhysicsSpring  mSphericalJointSpring;
    float               mSphericalProjectionDistance;
    MetroPhysicsLimit   mSphericalTwistLimitLow;
    MetroPhysicsLimit   mSphericalTwistLimitHigh;
    MetroPhysicsLimit   mSphericalSwingLimit;
    uint32_t            mSphericalFlags;
    vec3                mSphericalSwingAxis;
    uint32_t            mSphericalProjectionMode;
};

class MetroPhysicsJointPointOnLine final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointPointOnLine();
    virtual ~MetroPhysicsJointPointOnLine();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;
};

class MetroPhysicsJointPointInPlane final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointPointInPlane();
    virtual ~MetroPhysicsJointPointInPlane();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;
};

class MetroPhysicsJointDistance final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointDistance();
    virtual ~MetroPhysicsJointDistance();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;

private:
    float               mMinDistance;
    float               mMaxDistance;
    uint32_t            mDistanceFlags;
    MetroPhysicsSpring  mDistanceSpring;
};

class MetroPhysicsJointPulley final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointPulley();
    virtual ~MetroPhysicsJointPulley();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;

private:
    float               mPulleyDistance;
    float               mPulleyStiffness;
    float               mPulleyRatio;
    uint32_t            mPulleyFlags;
    MetroPhysicsMotor   mPulleyMotor;
    vec3                mPulley[2];
};

class MetroPhysicsJointFixed final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointFixed();
    virtual ~MetroPhysicsJointFixed();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;
};

class MetroPhysicsJointD6 final : public MetroPhysicsJoint {
    INHERITED_CLASS(MetroPhysicsJoint)
public:
    MetroPhysicsJointD6();
    virtual ~MetroPhysicsJointD6();

    virtual bool Load(NxuBinaryStream* stream) override;
    virtual JointType Type() const override;

private:
    void ReadJointSoftLimit(NxuBinaryStream* stream, SoftLimit& limit);
    void ReadJointDrive(NxuBinaryStream* stream, JointDrive& drive);

private:
    uint32_t        mXMotion;
    uint32_t        mYMotion;
    uint32_t        mZMotion;
    uint32_t        mSwing1Motion;
    uint32_t        mSwing2Motion;
    uint32_t        mTwistMotion;
    SoftLimit       mLinearLimit;
    SoftLimit       mSwing1Limit;
    SoftLimit       mSwing2Limit;
    SoftPairLimit   mTwistLimit;
    JointDrive      mXDrive;
    JointDrive      mYDrive;
    JointDrive      mZDrive;
    JointDrive      mSwingDrive;
    JointDrive      mTwistDrive;
    JointDrive      mSlerpDrive;
    vec3            mDrivePosition;
    quat            mDriveOrientation;
    vec3            mDriveLinearVelocity;
    vec3            mDriveAngularVelocity;
    uint32_t        mProjectionMode;
    float           mProjectionDistance;
    float           mProjectionAngle;
    float           mGearRatio;
    uint32_t        mD6Flags;
};


class MetroPhysicsScene {
public:
    MetroPhysicsScene();
    ~MetroPhysicsScene();

    bool                Load(NxuBinaryStream* stream);

private:
    bool                mGroundPlane;
    bool                mBoundsPlanes;
    vec3                mGravity;
    uint32_t            mTimeStepMethod;
    float               mMaxTimestep;
    uint32_t            mMaxIter;
    uint32_t            mSimType;
    bool                mHasLimits;
    uint32_t            mLimitsMaxActors;
    uint32_t            mLimitsMaxBodies;
    uint32_t            mLimitsMaxStaticShapes;
    uint32_t            mLimitsMaxDynamicShapes;
    uint32_t            mLimitsMaxJoints;
    bool                mHasBounds;
    AABBox              mBounds;
    uint32_t            mInternalThreadCount;
    uint32_t            mThreadMask;
    MetroPhysicsModel*  mModel;
};
