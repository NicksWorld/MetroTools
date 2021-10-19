#include "MetroPhysics.h"

#include "metro/MetroTypes.h"

enum NxuSectionTypes {
    NxuTypeSDK = 0x0,
    NxuTypeScene = 0x1,
    NxuTypeModel = 0x2,
    NxuTypeActor = 0x3,
    NxuTypeJoint = 0x4,
    NxuTypeMaterial = 0x5,
    NxuTypePairFlag = 0x6,
    NxuTypeShape = 0x7,
    NxuTypeTriangleMesh = 0x8,
    NxuTypeConvexMesh = 0x9,
    NxuTypeCCDSkeleton = 0xA,
    NxuTypeEffector = 0xB,
    NxuTypeFluid = 0xC,
    NxuTypeCloth = 0xD,
    NxuTypeFluidEmitter = 0xE,
    NxuTypeHeightField = 0xF,
    NxuTypeModelInstance = 0x10,
    NxuTypeUserData = 0x11,
    NxuTypeCollisionGroup = 0x12,
    NxuTypeUnknown = 0x13
};

enum NxuFormatType {
    BINARY = 0,
    BINARY_LITTLE_END = 1,
    BINARY_BIG_END = 2,
    ASCII = 3,
    COLLADA = 4
};

class NxuBinaryStream {
public:
    NxuBinaryStream()
        : mStream(nullptr)
        , mStreamName{}
        , mStreamType(0)
        , mStreamFlags(0)
    {}
    ~NxuBinaryStream() {}

    void Start(MemStream* srcStream) {
        mStream = srcStream;
        mStreamType = mStream->ReadU8();
        mStreamFlags = mStream->ReadU32();
    }

    uint32_t OpenSection() {
        mStreamType = mStream->ReadU8();
        mStreamFlags = mStream->ReadU32();
        mStreamName = this->ReadNameN<64>();

        return mStreamType;
    }

    inline uint32_t GetStreamType() const {
        return mStreamType;
    }
    inline uint32_t GetStreamFlags() const {
        return mStreamFlags;
    }
    inline const CharString& GetStreamName() const {
        return mStreamName;
    }

    inline bool ReadBool() {
        return mStream->ReadBool();
    }
    inline uint32_t ReadWord() {
        return mStream->ReadU16();
    }
    inline uint32_t ReadDword() {
        return mStream->ReadU32();
    }
    inline float ReadFloat() {
        return mStream->ReadF32();
    }
    inline vec3 ReadVector() {
        vec3 v;
        mStream->ReadStruct(v);
        return v;
    }
    inline quat ReadQuat() {
        quat q;
        mStream->ReadStruct(q);
        return q;
    }
    inline mat4 ReadMatrix4() {
        mat4 m;
        m[0] = vec4(this->ReadVector(), 0.0f);
        m[1] = vec4(this->ReadVector(), 0.0f);
        m[2] = vec4(this->ReadVector(), 0.0f);
        m[3] = vec4(this->ReadVector(), 1.0f);
        return m;
    }
    inline void ReadBuffer(void* buffer, const size_t numBytes) {
        mStream->ReadToBuffer(buffer, numBytes);
    }
    inline void Skip(const size_t bytesToSkip) {
        mStream->SkipBytes(bytesToSkip);
    }

    template <size_t N>
    CharString ReadNameN() {
        char s_name[N];

        mStream->ReadToBuffer(s_name, sizeof(s_name));

        return CharString(s_name);
    }

private:
    MemStream*  mStream;
    CharString  mStreamName;
    uint32_t    mStreamType;
    uint32_t    mStreamFlags;
};

MetroPhysicsCollection* MetroPhysicsLoadCollectionFromStream(MemStream srcStream) {
    NxuBinaryStream stream;
    stream.Start(&srcStream);

    MetroPhysicsCollection* collection = new MetroPhysicsCollection;
    if (!collection->Load(&stream)) {
        MySafeDelete(collection);
    }

    return collection;
}

void MetroPhysicsSpring::Read(NxuBinaryStream* stream, MetroPhysicsSpring& spring) {
    spring.spring = stream->ReadFloat();
    spring.damper = stream->ReadFloat();
    spring.targetValue = stream->ReadFloat();
}

void MetroPhysicsLimit::Read(NxuBinaryStream* stream, MetroPhysicsLimit& limit) {
    limit.hardness = stream->ReadFloat();
    limit.restitution = stream->ReadFloat();
    limit.value = stream->ReadFloat();
}

void MetroPhysicsMotor::Read(NxuBinaryStream* stream, MetroPhysicsMotor& motor) {
    motor.freeSpin = stream->ReadBool();
    motor.maxForce = stream->ReadFloat();
    motor.velTarget = stream->ReadFloat();
}

void MetroPhysicsPairFlag::Read(NxuBinaryStream* stream, MetroPhysicsPairFlag& pairFlag) {
    pairFlag.flag = stream->ReadDword();
    pairFlag.actor0Index = stream->ReadDword();
    pairFlag.shape0Index = stream->ReadDword();
    pairFlag.actor1Index = stream->ReadDword();
    pairFlag.shape1Index = stream->ReadDword();
}

struct NxuParameter {
    uint32_t    param;
    float       value;
};


MetroPhysicsCollection::MetroPhysicsCollection() {
}
MetroPhysicsCollection::~MetroPhysicsCollection() {
}

bool MetroPhysicsCollection::Load(NxuBinaryStream* stream) {
    mDefaultModel = MakeStrongPtr<MetroPhysicsModel>();
    mDefaultActor = MakeStrongPtr<MetroPhysicsActor>();

    stream->Skip(24); // 16 bytes GUID + 8 bytes source file time

    uint32_t numParams = 0;
    uint32_t numConvexes = 0;
    uint32_t numTrimesh = 0;
    uint32_t numHeightfields = 0;
    uint32_t numSkeletons = 0;
    uint32_t numShapes = 0;
    uint32_t numMats = 0;
    uint32_t numActors = 0;
    uint32_t numJoints = 0;
    uint32_t numPairFlags = 0;
    uint32_t numEffectors = 0;
    uint32_t numFluids = 0;
    uint32_t numCloths = 0;
    uint32_t numModels = 0;
    uint32_t numScenes = 0;

    bool isContainer = false;

    const uint32_t streamFlags = stream->GetStreamFlags();

    const uint32_t sectionType = stream->OpenSection();
    if (sectionType == NxuTypeSDK) {
        numParams = stream->ReadDword();
        numConvexes = stream->ReadDword();
        numTrimesh = stream->ReadDword();
        numHeightfields = (streamFlags & 0x100) ? stream->ReadDword() : 0;
        numSkeletons = (streamFlags & 0x1) ? stream->ReadDword() : 0;
        numShapes = stream->ReadDword();
        numMats = stream->ReadDword();
        numActors = stream->ReadDword();
        numJoints = stream->ReadDword();
        numPairFlags = stream->ReadDword();
        numEffectors = stream->ReadDword();
        numFluids = (streamFlags & 0x400) ? stream->ReadDword() : 0;
        numCloths = (streamFlags & 0x800) ? stream->ReadDword() : 0;
        numModels = stream->ReadDword();
        numScenes = stream->ReadDword();

        isContainer = true;

        MyArray<NxuParameter> params(numParams);
        for (auto& p : params) {
            p.param = stream->ReadDword();
            p.value = stream->ReadFloat();
        }
    } else {
        switch (sectionType) {
            case NxuTypeScene: {
                numScenes = 1;
            } break;
            case NxuTypeModel: {
                numModels = 1;
            } break;
            case NxuTypeActor: {
                numActors = 1;
            } break;
            case NxuTypeJoint: {
                numJoints = 1;
            } break;
            case NxuTypeMaterial: {
                numMats = 1;
            } break;
            case NxuTypePairFlag: {
                numPairFlags = 1;
            } break;
            case NxuTypeShape: {
                numShapes = 1;
            } break;
            case NxuTypeTriangleMesh: {
                numTrimesh = 1;
            } break;
            case NxuTypeConvexMesh: {
                numConvexes = 1;
            } break;
            case NxuTypeEffector: {
                numEffectors = 1;
            } break;
        }
    }

    if (numConvexes) {
        mConvexes.reserve(numConvexes);
        for (size_t i = 0; i < numConvexes; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeConvexMesh) {
                assert(false);
            }

            MetroPhysicsConvex* convex = new MetroPhysicsConvex;
            convex->Load(stream);
            mConvexes.push_back(convex);
        }
    }

    if (numTrimesh) {
        mTrimeshes.reserve(numTrimesh);
        for (size_t i = 0; i < numTrimesh; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeTriangleMesh) {
                assert(false);
            }

            MetroPhysicsTrimesh* trimesh = new MetroPhysicsTrimesh;
            trimesh->Load(stream);
            mTrimeshes.push_back(trimesh);
        }
    }

    if (numHeightfields) {
        // read (2033 only ???)
    }

    if (numSkeletons) {
        // read (2033 only ???)
    }

    if (numShapes) {
        for (size_t i = 0; i < numShapes; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeShape) {
                assert(false);
            }

            MetroPhysicsShape* shape = MetroPhysicsShape::ReadShape(stream);
            assert(shape != nullptr);
            if (shape) {
                mDefaultActor->AddShape(shape);
            }
        }
    }

    if (numMats) {
        for (size_t i = 0; i < numMats; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeMaterial) {
                assert(false);
            }

            MetroPhysicsMaterial* material = new MetroPhysicsMaterial;
            material->Load(stream);
            mDefaultModel->AddMaterial(material);
        }
    }

    if (numActors) {
        for (size_t i = 0; i < numActors; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeActor) {
                assert(false);
            }

            MetroPhysicsActor* actor = new MetroPhysicsActor;
            actor->Load(stream);
            mDefaultModel->AddActor(actor);
        }
    }

    if (numJoints) {
        for (size_t i = 0; i < numJoints; ++i) {
            MetroPhysicsJoint* joint = MetroPhysicsJoint::ReadJoint(stream);
            joint->Load(stream);
            mDefaultModel->AddJoint(joint);
        }
    }

    if (numPairFlags) {
        for (size_t i = 0; i < numPairFlags; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypePairFlag) {
                assert(false);
            }

            MetroPhysicsPairFlag pairFlag;
            MetroPhysicsPairFlag::Read(stream, pairFlag);
            mDefaultModel->AddPairFlag(pairFlag);
        }
    }

    if (numEffectors) {
        for (size_t i = 0; i < numEffectors; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeEffector) {
                assert(false);
            }

            MetroPhysicsSpringAndDamperEffector* effector = new MetroPhysicsSpringAndDamperEffector;
            effector->Load(stream);
            mDefaultModel->AddEffector(effector);
        }
    }

    if (numFluids) {
        // read (2033 only ???)
    }

    if (numCloths) {
        // read (2033 only ???)
    }

    if (numModels) {
        mModels.reserve(numModels);
        for (size_t i = 0; i < numModels; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeModel) {
                assert(false);
            }

            MetroPhysicsModel* model = new MetroPhysicsModel;
            model->Load(stream);
            mModels.push_back(model);
        }
    }

    if (numScenes) {
        mScenes.reserve(numScenes);
        for (size_t i = 0; i < numScenes; ++i) {
            if (isContainer && stream->OpenSection() != NxuTypeScene) {
                assert(false);
            }

            MetroPhysicsScene* scene = new MetroPhysicsScene;
            scene->Load(stream);
            mScenes.push_back(scene);
        }
    }

    return true;
}



// Model

MetroPhysicsModel::MetroPhysicsModel() {
}
MetroPhysicsModel::~MetroPhysicsModel() {
}

bool MetroPhysicsModel::Load(NxuBinaryStream* stream) {
    mName = stream->ReadNameN<256>();

    const size_t matsCount = stream->ReadDword();
    const size_t actorsCount = stream->ReadDword();
    const size_t jointsCount = stream->ReadDword();
    const size_t pairFlagsCount = stream->ReadDword();
    const size_t effectorsCount = stream->ReadDword();
    const size_t collisionGroupsCount = stream->ReadDword();
    const size_t clothCount = stream->ReadDword();
    const size_t fluidCount = stream->ReadDword();
    const size_t modelsCount = stream->ReadDword();

    if (matsCount) {
        mMaterials.reserve(matsCount);
        for (size_t i = 0; i < matsCount; ++i) {
            const NxuFormatType type = scast<NxuFormatType>(stream->OpenSection());
            assert(type == NxuTypeMaterial);

            MetroPhysicsMaterial* material = new MetroPhysicsMaterial;
            material->Load(stream);
            mMaterials.push_back(material);
        }
    }

    if (actorsCount) {
        mActors.reserve(actorsCount);
        for (size_t i = 0; i < actorsCount; ++i) {
            const NxuFormatType type = scast<NxuFormatType>(stream->OpenSection());
            assert(type == NxuTypeActor);

            MetroPhysicsActor* actor = new MetroPhysicsActor;
            actor->Load(stream);
            mActors.push_back(actor);
        }
    }

    if (jointsCount) {
        mJoints.reserve(jointsCount);

        for (size_t i = 0; i < jointsCount; ++i) {
            MetroPhysicsJoint* joint = MetroPhysicsJoint::ReadJoint(stream);
            assert(joint != nullptr);
            mJoints.push_back(joint);
        }
    }

    if (pairFlagsCount) {
        mPairFlags.resize(pairFlagsCount);

        for (auto& pairFlag : mPairFlags) {
            const NxuFormatType type = scast<NxuFormatType>(stream->OpenSection());
            assert(type == NxuTypePairFlag);

            MetroPhysicsPairFlag::Read(stream, pairFlag);
        }
    }

    if (effectorsCount) {
        mEffectors.reserve(effectorsCount);

        for (size_t i = 0; i < effectorsCount; ++i) {
            const NxuFormatType type = scast<NxuFormatType>(stream->OpenSection());
            assert(type == NxuTypeEffector);

            MetroPhysicsSpringAndDamperEffector* effector = new MetroPhysicsSpringAndDamperEffector;
            effector->Load(stream);
            mEffectors.push_back(effector);
        }
    }

    if (collisionGroupsCount) {
        mCollisionGroupA.reserve(collisionGroupsCount);
        mCollisionGroupB.reserve(collisionGroupsCount);

        for (size_t i = 0; i < collisionGroupsCount; ++i) {
            const NxuFormatType type = scast<NxuFormatType>(stream->OpenSection());
            assert(type == NxuTypeCollisionGroup);

            mCollisionGroupA.push_back(stream->ReadDword());
            mCollisionGroupB.push_back(stream->ReadDword());
        }
    }

    if (fluidCount) {   // 2033 only !!!
        assert(false);
    }

    if (clothCount) {   // 2033 only !!!
        assert(false);
    }

    if (modelsCount) {
        mModels.reserve(modelsCount);

        for (size_t i = 0; i < modelsCount; ++i) {
            const NxuFormatType type = scast<NxuFormatType>(stream->OpenSection());
            assert(type == NxuTypeModelInstance);

            MetroPhysicsModelInstance* instance = new MetroPhysicsModelInstance;
            instance->Load(stream);
            mModels.push_back(instance);
        }
    }

    return true;
}

void MetroPhysicsModel::AddMaterial(MetroPhysicsMaterial* material) {
    mMaterials.push_back(material);
}

void MetroPhysicsModel::AddActor(MetroPhysicsActor* actor) {
    mActors.push_back(actor);
}

void MetroPhysicsModel::AddJoint(MetroPhysicsJoint* joint) {
    mJoints.push_back(joint);
}

void MetroPhysicsModel::AddPairFlag(const MetroPhysicsPairFlag& pairFlag) {
    mPairFlags.push_back(pairFlag);
}

void MetroPhysicsModel::AddEffector(MetroPhysicsSpringAndDamperEffector* effector) {
    mEffectors.push_back(effector);
}


// ModelInstance

MetroPhysicsModelInstance::MetroPhysicsModelInstance() {
}
MetroPhysicsModelInstance::~MetroPhysicsModelInstance() {
}

bool MetroPhysicsModelInstance::Load(NxuBinaryStream* stream) {
    mPose = stream->ReadMatrix4();
    mModelIdx = stream->ReadDword();

    return true;
}


// Effector

MetroPhysicsSpringAndDamperEffector::MetroPhysicsSpringAndDamperEffector()
    : mRefAttachActorDesc(0)
    , mAttachActorDesc(0)
    , mPos1()
    , mPos2()
    , mSpringDistCompressSaturate(0.0f)
    , mSpringDistRelaxed(0.0f)
    , mSpringDistStretchSaturate(0.0f)
    , mSpringMaxCompressForce(0.0f)
    , mSpringMaxStretchForce(0.0f)
    , mDamperVelCompressSaturate(0.0f)
    , mDamperVelStretchSaturate(0.0f)
    , mDamperMaxCompressForce(0.0f)
    , mDamperMaxStretchForce(0.0f)
{
}
MetroPhysicsSpringAndDamperEffector::~MetroPhysicsSpringAndDamperEffector() {
}

bool MetroPhysicsSpringAndDamperEffector::Load(NxuBinaryStream* stream) {
    mRefAttachActorDesc = stream->ReadDword();
    mAttachActorDesc = stream->ReadDword();
    mPos1 = stream->ReadVector();
    mPos2 = stream->ReadVector();
    mSpringDistCompressSaturate = stream->ReadFloat();
    mSpringDistRelaxed = stream->ReadFloat();
    mSpringDistStretchSaturate = stream->ReadFloat();
    mSpringMaxCompressForce = stream->ReadFloat();
    mSpringMaxStretchForce = stream->ReadFloat();
    mDamperVelCompressSaturate = stream->ReadFloat();
    mDamperVelStretchSaturate = stream->ReadFloat();
    mDamperMaxCompressForce = stream->ReadFloat();
    mDamperMaxStretchForce = stream->ReadFloat();

    return true;
}


// Convex

MetroPhysicsConvex::MetroPhysicsConvex()
    : mFlags(0)
    , mNumVertices(0)
    , mVertexSize(0)
    , mNumTriangles(0)
    , mTriangleSize(0)
{
}
MetroPhysicsConvex::~MetroPhysicsConvex() {

}

bool MetroPhysicsConvex::Load(NxuBinaryStream* stream) {
    mFlags = stream->ReadDword();
    mNumVertices = stream->ReadDword();
    mVertexSize = stream->ReadDword();
    mVerticesData.resize(scast<size_t>(mNumVertices) * mVertexSize);
    stream->ReadBuffer(mVerticesData.data(), mVerticesData.size());

    mNumTriangles = stream->ReadDword();
    mTriangleSize = stream->ReadDword();
    mTrianglesData.resize(scast<size_t>(mNumTriangles) * mTriangleSize);
    stream->ReadBuffer(mTrianglesData.data(), mTrianglesData.size());

    const size_t cookedDataSize = stream->ReadDword();
    mCookedData.resize(cookedDataSize);
    stream->ReadBuffer(mCookedData.data(), mCookedData.size());

    return true;
}

bool MetroPhysicsConvex::AreIndices16Bit() const {
    return (mFlags & 2) != 0;
}


// Trimesh 

MetroPhysicsTrimesh::MetroPhysicsTrimesh()
    : mHasMaterialIndicies(false)
    , mHeightFieldVerticalAxis(0)
    , mHeightFieldVerticalExtent(0.0f)
    , mConvexEdgeThreshold(0.0f)
    , mFlags(0)
    , mNumVertices(0)
    , mVertexSize(0)
    , mNumTriangles(0)
    , mTriangleSize(0)
{
}
MetroPhysicsTrimesh::~MetroPhysicsTrimesh() {

}

bool MetroPhysicsTrimesh::Load(NxuBinaryStream* stream) {
    mHasMaterialIndicies = stream->ReadBool();
    mHeightFieldVerticalAxis = stream->ReadDword();
    mHeightFieldVerticalExtent = stream->ReadFloat();
    mConvexEdgeThreshold = stream->ReadFloat();

    mFlags = stream->ReadDword();

    mNumVertices = stream->ReadDword();
    mVertexSize = stream->ReadDword();
    mVerticesData.resize(scast<size_t>(mNumVertices) * mVertexSize);
    stream->ReadBuffer(mVerticesData.data(), mVerticesData.size());

    mNumTriangles = stream->ReadDword();
    mTriangleSize = stream->ReadDword();
    mTrianglesData.resize(scast<size_t>(mNumTriangles) * mTriangleSize);
    stream->ReadBuffer(mTrianglesData.data(), mVerticesData.size());

    const size_t cookedDataSize = stream->ReadDword();
    mCookedData.resize(cookedDataSize);
    stream->ReadBuffer(mCookedData.data(), mCookedData.size());

    const size_t pmapDataSize = stream->ReadDword();
    mPmap.resize(pmapDataSize);
    stream->ReadBuffer(mPmap.data(), mPmap.size());

    return true;
}



// Material

MetroPhysicsMaterial::MetroPhysicsMaterial() {
}
MetroPhysicsMaterial::~MetroPhysicsMaterial() {
}

bool MetroPhysicsMaterial::Load(NxuBinaryStream* stream) {
    mIndex = stream->ReadWord();
    mDynamicFriction = stream->ReadFloat();
    mStaticFriction = stream->ReadFloat();
    mRestitution = stream->ReadFloat();
    mDynamicFrictionV = stream->ReadFloat();
    mStaticFrictionV = stream->ReadFloat();
    mFrictionCombineMode = stream->ReadDword();
    mRestitutionCombineMode = stream->ReadDword();
    mDirOfAnisotropy = stream->ReadVector();
    mFlags = stream->ReadDword();
    mHasSpring = stream->ReadBool();
    if (mHasSpring) {
        mSpringSpring = stream->ReadFloat();
        mSpringDamper = stream->ReadFloat();
        mSpringTargetValue = stream->ReadFloat();
    }

    return true;
}



// Actor

MetroPhysicsActor::MetroPhysicsActor() {
}
MetroPhysicsActor::~MetroPhysicsActor() {
}

bool MetroPhysicsActor::Load(NxuBinaryStream* stream) {
    mGlobalPose = stream->ReadMatrix4();

    mHasBody = stream->ReadBool();
    if (mHasBody) {
        mBodyMassLocalPose = stream->ReadMatrix4();
        mBodyMassSpaceInertia = stream->ReadVector();
        mBodyMass = stream->ReadFloat();
        mBodyLinearVelocity = stream->ReadVector();
        mBodyAngularVelocity = stream->ReadVector();
        mBodyWakeUpCounter = stream->ReadFloat();
        mBodyLinearDamping = stream->ReadFloat();
        mBodyAngularDamping = stream->ReadFloat();
        mBodyMaxAngularVelocity = stream->ReadFloat();
        mBodyCCDMotionThreshold = stream->ReadFloat();  // later versions of the game are checking for the flag 2
        mBodyFlags = stream->ReadDword();
        mBodySleepLinearVelocity = stream->ReadFloat();
        mBodySleepAngularVelocity = stream->ReadFloat();
        mBodySolverIterationCount = stream->ReadDword();
    }

    mDensity = stream->ReadFloat();
    mGroup = stream->ReadDword();
    mFlags = stream->ReadDword();
    mUserData = stream->ReadDword();

    mName = stream->ReadNameN<256>();

    const size_t numShapes = stream->ReadDword();
    if (numShapes) {
        mShapes.reserve(numShapes);
        for (size_t i = 0; i < numShapes; ++i) {
            MetroPhysicsShape* shape = MetroPhysicsShape::ReadShape(stream);
            assert(shape != nullptr);
            if (shape) {
                mShapes.push_back(shape);
            }
        }
    }

    return true;
}

void MetroPhysicsActor::AddShape(MetroPhysicsShape* shape) {
    mShapes.push_back(shape);
}



// Shapes
#include "MetroPhysicsShapes.inl"


// Joints
#include "MetroPhysicsJoints.inl"



// Scene
MetroPhysicsScene::MetroPhysicsScene()
    : mModel(nullptr)
{
}
MetroPhysicsScene::~MetroPhysicsScene() {
    MySafeDelete(mModel);
}

bool MetroPhysicsScene::Load(NxuBinaryStream* stream) {
    mGroundPlane = stream->ReadBool();
    mBoundsPlanes = stream->ReadBool();

    mGravity = stream->ReadVector();

    mTimeStepMethod = stream->ReadDword();
    mMaxTimestep = stream->ReadFloat();
    mMaxIter = stream->ReadDword();

    mSimType = 0;
    // if 2033 - read always
    //if (stream->GetStreamFlags() & 4) {
        mSimType = stream->ReadDword();
    //}

    mHasLimits = stream->ReadBool();
    if (mHasLimits) {
        mLimitsMaxActors = stream->ReadDword();
        mLimitsMaxBodies = stream->ReadDword();
        mLimitsMaxStaticShapes = stream->ReadDword();
        mLimitsMaxDynamicShapes = stream->ReadDword();
        mLimitsMaxJoints = stream->ReadDword();
    }

    mHasBounds = stream->ReadBool();
    if (mHasBounds) {
        mBounds.minimum = stream->ReadVector();
        mBounds.maximum = stream->ReadVector();
    }

    mInternalThreadCount = 0;
    mThreadMask = 0;
    // if 2033 - read always
    //if (stream->GetStreamFlags() & 0x80) {
        mInternalThreadCount = stream->ReadDword();
        mThreadMask = stream->ReadDword();
    //}

    // now read the contents, which is just a model
    const uint32_t sectionType = stream->OpenSection();
    assert(sectionType == NxuTypeModel);
    mModel = new MetroPhysicsModel;
    const bool result = mModel->Load(stream);

    return result;
}
