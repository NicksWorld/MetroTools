MetroPhysicsShape* MetroPhysicsShape::ReadShape(NxuBinaryStream* stream) {
    MetroPhysicsShape* shape = nullptr;

    stream->OpenSection();

    const MetroPhysicsShape::ShapeType shapeType = scast<MetroPhysicsShape::ShapeType>(stream->ReadDword());
    switch (shapeType) {
        case MetroPhysicsShape::ShapeType::Plane:
            shape = new MetroPhysicsShapePlane;
            break;
        case MetroPhysicsShape::ShapeType::Sphere:
            shape = new MetroPhysicsShapeSphere;
            break;
        case MetroPhysicsShape::ShapeType::Box:
            shape = new MetroPhysicsShapeBox;
            break;
        case MetroPhysicsShape::ShapeType::Capsule:
            shape = new MetroPhysicsShapeCapsule;
            break;
        case MetroPhysicsShape::ShapeType::Wheel:
            shape = new MetroPhysicsShapeWheel;
            break;
        case MetroPhysicsShape::ShapeType::Convex:
            shape = new MetroPhysicsShapeConvex;
            break;
        case MetroPhysicsShape::ShapeType::TriMesh:
            shape = new MetroPhysicsShapeTrimesh;
            break;
        case MetroPhysicsShape::ShapeType::Heightfield:
            shape = new MetroPhysicsShapeHeightfield;
            break;
        default:
            break;
    }

    if (shape) {
        shape->Load(stream);
    }

    return shape;
}

bool MetroPhysicsShape::Load(NxuBinaryStream* stream) {
    //const uint32_t type = stream->ReadDword();
    //assert(type == scast<uint32_t>(this->Type()));

    mLocalPose = stream->ReadMatrix4();

    mGroup = stream->ReadDword();
    mMaterialIndex = stream->ReadDword();

    const uint32_t streamFlags = stream->GetStreamFlags();

    //if (streamFlags & 0xC) // on versions > 2033
    {
        mMass = stream->ReadFloat();
        mDensity = stream->ReadFloat();
    }
    mSkinWidth = stream->ReadFloat();

    mShapeFlags = stream->ReadDword();

    //if (streamFlags & 0x1) // on versions > 2033
    {
        mCCDSkeleton = stream->ReadDword();
    }

    if (streamFlags & 0xD) { // not on 2033 !!!
        mGroupsMask.dwords[0] = stream->ReadDword();
        mGroupsMask.dwords[1] = stream->ReadDword();
        mGroupsMask.dwords[2] = stream->ReadDword();
        mGroupsMask.dwords[3] = stream->ReadDword();
    }

    mName = stream->ReadNameN<256>();

    return true;
}

// Plane
MetroPhysicsShapePlane::MetroPhysicsShapePlane() {
}
MetroPhysicsShapePlane::~MetroPhysicsShapePlane() {
}

bool MetroPhysicsShapePlane::Load(NxuBinaryStream* stream) {
    return Base::Load(stream);
}

MetroPhysicsShape::ShapeType MetroPhysicsShapePlane::Type() const {
    return MetroPhysicsShape::ShapeType::Plane;
}

// Sphere
MetroPhysicsShapeSphere::MetroPhysicsShapeSphere() {
}
MetroPhysicsShapeSphere::~MetroPhysicsShapeSphere() {
}

bool MetroPhysicsShapeSphere::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mRadius = stream->ReadFloat();

    return mRadius;
}

MetroPhysicsShape::ShapeType MetroPhysicsShapeSphere::Type() const {
    return MetroPhysicsShape::ShapeType::Sphere;
}

// Box
MetroPhysicsShapeBox::MetroPhysicsShapeBox() {
}
MetroPhysicsShapeBox::~MetroPhysicsShapeBox() {
}

bool MetroPhysicsShapeBox::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mDimensions = stream->ReadVector();

    return result;
}

MetroPhysicsShape::ShapeType MetroPhysicsShapeBox::Type() const {
    return MetroPhysicsShape::ShapeType::Box;
}

// Capsule
MetroPhysicsShapeCapsule::MetroPhysicsShapeCapsule() {
}
MetroPhysicsShapeCapsule::~MetroPhysicsShapeCapsule() {
}

bool MetroPhysicsShapeCapsule::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mRadius = stream->ReadFloat();
    mHeight = stream->ReadFloat();
    mFlags = stream->ReadDword();

    return result;
}

MetroPhysicsShape::ShapeType MetroPhysicsShapeCapsule::Type() const {
    return MetroPhysicsShape::ShapeType::Capsule;
}

// Wheel (2033 only ???)
MetroPhysicsShapeWheel::MetroPhysicsShapeWheel() {
}
MetroPhysicsShapeWheel::~MetroPhysicsShapeWheel() {
}

bool MetroPhysicsShapeWheel::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mRadius = stream->ReadFloat();
    mSuspensionTravel = stream->ReadFloat();
    mSuspensionSpringSpring = stream->ReadFloat();
    mSuspensionSpringDamper = stream->ReadFloat();
    mSuspensionSpringTargetValue = stream->ReadFloat();

    this->ReadTireFunction(stream, mLongitudalTireForceFunction);
    this->ReadTireFunction(stream, mLateralTireForceFunction);

    mInverseWheelMass = stream->ReadFloat();
    mWheelFlags = stream->ReadDword();
    mMotorTorque = stream->ReadFloat();
    mBrakeTorque = stream->ReadFloat();
    mSteerAngle = stream->ReadFloat();

    return result;
}

MetroPhysicsShape::ShapeType MetroPhysicsShapeWheel::Type() const {
    return MetroPhysicsShape::ShapeType::Wheel;
}

void MetroPhysicsShapeWheel::ReadTireFunction(NxuBinaryStream* stream, TireFunction& tireFunc) {
    tireFunc.mExtremumSlip = stream->ReadFloat();
    tireFunc.mExtremumValue = stream->ReadFloat();
    tireFunc.mAsymptoteSlip = stream->ReadFloat();
    tireFunc.mAsymptoteValue = stream->ReadFloat();
    tireFunc.mStiffnessFactor = stream->ReadFloat();

}

// Convex
MetroPhysicsShapeConvex::MetroPhysicsShapeConvex() {
}
MetroPhysicsShapeConvex::~MetroPhysicsShapeConvex() {
}

bool MetroPhysicsShapeConvex::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mConvexMeshDesc = stream->ReadDword();
    mMeshFlags = stream->ReadDword();

    if (stream->GetStreamFlags() & 0xE) {   // not in 2033 !!!
        mScale = stream->ReadFloat();
    }

    return result;
}

MetroPhysicsShape::ShapeType MetroPhysicsShapeConvex::Type() const {
    return MetroPhysicsShape::ShapeType::Convex;
}

// Trimesh
MetroPhysicsShapeTrimesh::MetroPhysicsShapeTrimesh() {
}
MetroPhysicsShapeTrimesh::~MetroPhysicsShapeTrimesh() {
}

bool MetroPhysicsShapeTrimesh::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mTrimeshMeshDesc = stream->ReadDword();
    mMeshFlags = stream->ReadDword();

    if (stream->GetStreamFlags() & 0xE) {   // not in 2033 !!!
        mScale = stream->ReadFloat();
    }

    return result;
}

MetroPhysicsShape::ShapeType MetroPhysicsShapeTrimesh::Type() const {
    return MetroPhysicsShape::ShapeType::TriMesh;
}

// Heightfield (2033 only ???)
MetroPhysicsShapeHeightfield::MetroPhysicsShapeHeightfield() {
}
MetroPhysicsShapeHeightfield::~MetroPhysicsShapeHeightfield() {
}

bool MetroPhysicsShapeHeightfield::Load(NxuBinaryStream* stream) {
    const bool result = Base::Load(stream);

    mHeightFieldDesc = stream->ReadDword();
    mHeightScale = stream->ReadFloat();
    mRowScale = stream->ReadFloat();
    mColumnScale = stream->ReadFloat();
    mMaterialIndexHighBits = stream->ReadWord();
    mHoleMaterial = stream->ReadWord();
    mMeshFlags = stream->ReadDword();

    return result;
}

MetroPhysicsShape::ShapeType MetroPhysicsShapeHeightfield::Type() const {
    return MetroPhysicsShape::ShapeType::Heightfield;
}
