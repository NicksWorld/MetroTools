MetroPhysicsJoint* MetroPhysicsJoint::ReadJoint(NxuBinaryStream* stream) {
    const uint32_t sectionType = stream->OpenSection();
    assert(sectionType == NxuTypeJoint);

    const JointType type = scast<JointType>(stream->ReadDword());

    MetroPhysicsJoint* joint = nullptr;
    switch (type) {
        case JointType::Prismatic: {
            joint = new MetroPhysicsJointPrismatic;
        } break;
        case JointType::Revolute: {
            joint = new MetroPhysicsJointRevolute;
        } break;
        case JointType::Cylindrical: {
            joint = new MetroPhysicsJointCylindrical;
        } break;
        case JointType::Spherical: {
            joint = new MetroPhysicsJointSpherical;
        } break;
        case JointType::PointOnLine: {
            joint = new MetroPhysicsJointPointOnLine;
        } break;
        case JointType::PointInPlane: {
            joint = new MetroPhysicsJointPointInPlane;
        } break;
        case JointType::Distance: {
            joint = new MetroPhysicsJointDistance;
        } break;
        case JointType::Pulley: {
            joint = new MetroPhysicsJointPulley;
        } break;
        case JointType::Fixed: {
            joint = new MetroPhysicsJointFixed;
        } break;
        case JointType::D6: {
            joint = new MetroPhysicsJointD6;
        } break;

        default: {
            joint = nullptr;
        } break;
    }

    if (joint) {
        joint->Load(stream);
    }

    return joint;
}

MetroPhysicsJoint::MetroPhysicsJoint() {
}
MetroPhysicsJoint::~MetroPhysicsJoint() {
}

bool MetroPhysicsJoint::Load(NxuBinaryStream* stream) {
    mRefAttachActorDesc = stream->ReadDword();
    mAttachActorDesc = stream->ReadDword();
    mLocalNormal0 = stream->ReadVector();
    mLocalAxis0 = stream->ReadVector();
    mLocalAnchor0 = stream->ReadVector();
    mLocalNormal1 = stream->ReadVector();
    mLocalAxis1 = stream->ReadVector();
    mLocalAnchor1 = stream->ReadVector();
    mMaxForce = stream->ReadFloat();
    mMaxTorque = stream->ReadFloat();
    mName = stream->ReadNameN<256>();
    mJointFlags = stream->ReadDword();
    mNumLimitPlanes = stream->ReadDword();
    mPlaneLimitPoint = stream->ReadVector();
    mOnActor2 = stream->ReadBool();

    if (mNumLimitPlanes) {
        mLimitPlanes.resize(mNumLimitPlanes);

        for (auto& lp : mLimitPlanes) {
            lp.normal = stream->ReadVector();
            lp.distance = stream->ReadFloat();
            lp.worldLimitPt = stream->ReadVector();
        }
    }

    return true;
}

// Prismatic
MetroPhysicsJointPrismatic::MetroPhysicsJointPrismatic() {
}
MetroPhysicsJointPrismatic::~MetroPhysicsJointPrismatic() {
}

bool MetroPhysicsJointPrismatic::Load(NxuBinaryStream* stream) {
    return Base::Load(stream);
}

MetroPhysicsJoint::JointType MetroPhysicsJointPrismatic::Type() const {
    return MetroPhysicsJoint::JointType::Prismatic;
}

// Revolute
MetroPhysicsJointRevolute::MetroPhysicsJointRevolute() {
}
MetroPhysicsJointRevolute::~MetroPhysicsJointRevolute() {
}

bool MetroPhysicsJointRevolute::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    MetroPhysicsLimit::Read(stream, mRevoluteLimitLow);
    MetroPhysicsLimit::Read(stream, mRevoluteLimitHigh);

    MetroPhysicsMotor::Read(stream, mRevoluteMotor);

    MetroPhysicsSpring::Read(stream, mRevoluteSpring);

    mRevoluteProjectionDistance = stream->ReadFloat();
    mRevoluteProjectionAngle = stream->ReadFloat();
    mRevoluteFlags = stream->ReadDword();
    mRevoluteProjectionMode = stream->ReadDword();

    return result;
}

MetroPhysicsJoint::JointType MetroPhysicsJointRevolute::Type() const {
    return MetroPhysicsJoint::JointType::Revolute;
}

// Cylindrical
MetroPhysicsJointCylindrical::MetroPhysicsJointCylindrical() {
}
MetroPhysicsJointCylindrical::~MetroPhysicsJointCylindrical() {
}

bool MetroPhysicsJointCylindrical::Load(NxuBinaryStream* stream) {
    return Base::Load(stream);
}

MetroPhysicsJoint::JointType MetroPhysicsJointCylindrical::Type() const {
    return MetroPhysicsJoint::JointType::Cylindrical;
}

// Spherical
MetroPhysicsJointSpherical::MetroPhysicsJointSpherical() {
}
MetroPhysicsJointSpherical::~MetroPhysicsJointSpherical() {
}

bool MetroPhysicsJointSpherical::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    MetroPhysicsSpring::Read(stream, mSphericalTwistSpring);
    MetroPhysicsSpring::Read(stream, mSphericalSwingSpring);
    MetroPhysicsSpring::Read(stream, mSphericalJointSpring);

    mSphericalProjectionDistance = stream->ReadFloat();

    MetroPhysicsLimit::Read(stream, mSphericalTwistLimitLow);
    MetroPhysicsLimit::Read(stream, mSphericalTwistLimitHigh);
    MetroPhysicsLimit::Read(stream, mSphericalSwingLimit);

    mSphericalFlags = stream->ReadDword();
    mSphericalSwingAxis = stream->ReadVector();
    mSphericalProjectionMode = stream->ReadDword();

    return result;
}

MetroPhysicsJoint::JointType MetroPhysicsJointSpherical::Type() const {
    return MetroPhysicsJoint::JointType::Spherical;
}

// PointOnLine
MetroPhysicsJointPointOnLine::MetroPhysicsJointPointOnLine() {
}
MetroPhysicsJointPointOnLine::~MetroPhysicsJointPointOnLine() {
}

bool MetroPhysicsJointPointOnLine::Load(NxuBinaryStream* stream) {
    return Base::Load(stream);
}

MetroPhysicsJoint::JointType MetroPhysicsJointPointOnLine::Type() const {
    return MetroPhysicsJoint::JointType::PointOnLine;
}

// PointInPlane
MetroPhysicsJointPointInPlane::MetroPhysicsJointPointInPlane() {
}
MetroPhysicsJointPointInPlane::~MetroPhysicsJointPointInPlane() {
}

bool MetroPhysicsJointPointInPlane::Load(NxuBinaryStream* stream) {
    return Base::Load(stream);
}

MetroPhysicsJoint::JointType MetroPhysicsJointPointInPlane::Type() const {
    return MetroPhysicsJoint::JointType::PointInPlane;
}

// Distance
MetroPhysicsJointDistance::MetroPhysicsJointDistance() {
}
MetroPhysicsJointDistance::~MetroPhysicsJointDistance() {
}

bool MetroPhysicsJointDistance::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mMinDistance = stream->ReadFloat();
    mMaxDistance = stream->ReadFloat();
    mDistanceFlags = stream->ReadDword();

    MetroPhysicsSpring::Read(stream, mDistanceSpring);

    return result;
}

MetroPhysicsJoint::JointType MetroPhysicsJointDistance::Type() const {
    return MetroPhysicsJoint::JointType::Distance;
}

// Pulley
MetroPhysicsJointPulley::MetroPhysicsJointPulley() {
}
MetroPhysicsJointPulley::~MetroPhysicsJointPulley() {
}

bool MetroPhysicsJointPulley::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mPulleyDistance = stream->ReadFloat();
    mPulleyStiffness = stream->ReadFloat();
    mPulleyRatio = stream->ReadFloat();
    mPulleyFlags = stream->ReadDword();

    MetroPhysicsMotor::Read(stream, mPulleyMotor);

    mPulley[0] = stream->ReadVector();
    mPulley[1] = stream->ReadVector();

    return result;
}

MetroPhysicsJoint::JointType MetroPhysicsJointPulley::Type() const {
    return MetroPhysicsJoint::JointType::Pulley;
}

// Fixed
MetroPhysicsJointFixed::MetroPhysicsJointFixed() {
}
MetroPhysicsJointFixed::~MetroPhysicsJointFixed() {
}

bool MetroPhysicsJointFixed::Load(NxuBinaryStream* stream) {
    return Base::Load(stream);
}

MetroPhysicsJoint::JointType MetroPhysicsJointFixed::Type() const {
    return MetroPhysicsJoint::JointType::Fixed;
}

// D6
MetroPhysicsJointD6::MetroPhysicsJointD6() {
}
MetroPhysicsJointD6::~MetroPhysicsJointD6() {
}

bool MetroPhysicsJointD6::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mXMotion = stream->ReadDword();
    mYMotion = stream->ReadDword();
    mZMotion = stream->ReadDword();
    mSwing1Motion = stream->ReadDword();
    mSwing2Motion = stream->ReadDword();
    mTwistMotion = stream->ReadDword();

    this->ReadJointSoftLimit(stream, mLinearLimit);
    this->ReadJointSoftLimit(stream, mSwing1Limit);
    this->ReadJointSoftLimit(stream, mSwing2Limit);
    this->ReadJointSoftLimit(stream, mTwistLimit.low);
    this->ReadJointSoftLimit(stream, mTwistLimit.high);

    this->ReadJointDrive(stream, mXDrive);
    this->ReadJointDrive(stream, mYDrive);
    this->ReadJointDrive(stream, mZDrive);
    this->ReadJointDrive(stream, mSwingDrive);
    this->ReadJointDrive(stream, mTwistDrive);
    this->ReadJointDrive(stream, mSlerpDrive);

    mDrivePosition = stream->ReadVector();
    mDriveOrientation = stream->ReadQuat();
    mDriveLinearVelocity = stream->ReadVector();
    mDriveAngularVelocity = stream->ReadVector();

    mProjectionMode = stream->ReadDword();
    mProjectionDistance = stream->ReadFloat();
    mProjectionAngle = stream->ReadFloat();

    mGearRatio = stream->ReadFloat();

    mD6Flags = stream->ReadDword();

    return result;
}

MetroPhysicsJoint::JointType MetroPhysicsJointD6::Type() const {
    return MetroPhysicsJoint::JointType::D6;
}

void MetroPhysicsJointD6::ReadJointSoftLimit(NxuBinaryStream* stream, MetroPhysicsJoint::SoftLimit& limit) {
    limit.value = stream->ReadFloat();
    limit.restitution = stream->ReadFloat();
    limit.spring = stream->ReadFloat();
    limit.damping = stream->ReadFloat();
}

void MetroPhysicsJointD6::ReadJointDrive(NxuBinaryStream* stream, JointDrive& drive) {
    drive.driveType = stream->ReadDword();
    drive.spring = stream->ReadFloat();
    drive.damping = stream->ReadFloat();
    drive.forceLimit = stream->ReadFloat();
}
