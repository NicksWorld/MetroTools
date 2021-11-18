MetroPhysicsCForm::MetroPhysicsCForm() {
}
MetroPhysicsCForm::~MetroPhysicsCForm() {
}

uint32_t MetroPhysicsCForm::GameVersionToCFormVersion(const MetroGameVersion gameVersion) {
    // Static models:
    //  2033 formatVer == 5
    //  LL/Redux formatVer == 8
    //  A.1 formatVer == 10
    //  Exodus formatVer == 11
    //
    // LevelGeo:
    //  TODO


    if (MetroGameVersion::OG2033 == gameVersion) {
        return 5;
    } else if (MetroGameVersion::OGLastLight == gameVersion || MetroGameVersion::Redux == gameVersion) {
        return 8;
    } else if (MetroGameVersion::Arktika1 == gameVersion) {
        return 10;
    } else {
        return 11;
    }
}

bool MetroPhysicsCForm::Read(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) {
    const size_t numMeshes = stream->ReadDword();

    if (numMeshes) {
        mMeshes.reserve(numMeshes);
        for (size_t i = 0; i < numMeshes; ++i) {
            RefPtr<MetroPhysicsCMesh> mesh = MakeRefPtr<MetroPhysicsCMesh>();
            if (mesh->Read(stream, formatVer, isLevelGeo)) {
                mMeshes.push_back(mesh);
            }
        }
    }

    return mMeshes.size() == numMeshes;
}

void MetroPhysicsCForm::Write(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) const {
    const uint32_t numMeshes = scast<uint32_t>(mMeshes.size());
    stream->WriteDword(numMeshes);

    for (const auto& mesh : mMeshes) {
        mesh->Write(stream, formatVer, isLevelGeo);
    }
}

size_t MetroPhysicsCForm::GetNumCMeshes() const {
    return mMeshes.size();
}

RefPtr<MetroPhysicsCMesh> MetroPhysicsCForm::GetCMesh(const size_t idx) const {
    return mMeshes[idx];
}

void MetroPhysicsCForm::AddCMesh(const RefPtr<MetroPhysicsCMesh>& mesh) {
    mMeshes.push_back(mesh);
}


MetroPhysicsCMesh::MetroPhysicsCMesh()
    : mDummy(0)
    , mSector(0xFFFF)
    , mCollisionGroup(0)
    , mIsDummy(false)
    , mIsPrimitive(false)
    , mIsRaycast(true)
{
}
MetroPhysicsCMesh::~MetroPhysicsCMesh() {
}

bool MetroPhysicsCMesh::Read(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) {
    if (!this->ReadMaterial(stream, formatVer, isLevelGeo)) {
        return false;
    }

    // cooked NXTriMesh data
    const size_t cookedDataSize = stream->ReadDword();
    mCookedData.resize(cookedDataSize);
    stream->ReadBuffer(mCookedData.data(), cookedDataSize);

    return true;
}

void MetroPhysicsCMesh::Write(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) const {
    this->WriteMaterial(stream, formatVer, isLevelGeo);

    // cooked NXTriMesh data
    const uint32_t cookedDataSize = scast<uint32_t>(mCookedData.size());
    stream->WriteDword(cookedDataSize);
    stream->WriteBuffer(mCookedData.data(), cookedDataSize);
}

uint16_t MetroPhysicsCMesh::GetSector() const {
    return mSector;
}

void MetroPhysicsCMesh::SetSector(const uint16_t sector) {
    mSector = sector;
}

uint16_t MetroPhysicsCMesh::GetCollisionGroup() const {
    return mCollisionGroup;
}

void MetroPhysicsCMesh::SetCollisionGroup(const uint16_t colGroup) {
    mCollisionGroup = colGroup;
}

bool MetroPhysicsCMesh::IsPrimitive() const {
    return mIsPrimitive;
}

void MetroPhysicsCMesh::SetIsPrimitive(const bool b) {
    mIsPrimitive = b;
}

bool MetroPhysicsCMesh::IsRaycast() const {
    return mIsRaycast;
}

void MetroPhysicsCMesh::SetIsRaycast(const bool b) {
    mIsRaycast = b;
}

const CharString& MetroPhysicsCMesh::GetShader() const {
    return mShader;
}

void MetroPhysicsCMesh::SetShader(const CharString& shader) {
    mShader = shader;
}

const CharString& MetroPhysicsCMesh::GetTexture() const {
    return mTexList;
}

void MetroPhysicsCMesh::SetTexture(const CharString& texture) {
    mTexList = texture;
}

const CharString& MetroPhysicsCMesh::GetMaterialName1() const {
    return mMaterialName1;
}

void MetroPhysicsCMesh::SetMaterialName1(const CharString& material) {
    mMaterialName1 = material;
}

const CharString& MetroPhysicsCMesh::GetMaterialName2() const {
    return mMaterialName2;
}

void MetroPhysicsCMesh::SetMaterialName2(const CharString& material) {
    mMaterialName2 = material;
}

BytesArray& MetroPhysicsCMesh::GetCookedData() {
    return mCookedData;
}

void MetroPhysicsCMesh::SetCookedData(const void* data, const size_t dataSize) {
    mCookedData.resize(dataSize);
    std::memcpy(mCookedData.data(), data, dataSize);
}

bool MetroPhysicsCMesh::ReadMaterial(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) {
    mIsDummy = stream->ReadBool();
    if (mIsDummy) {
        mDummy = stream->ReadWord();
        return true;
    }

    bool hasMtlName = false;
    bool hasSector = false;
    bool hasColGroup = false;
    bool hasRaycast = false;
    bool hasPrimitive = true;
    bool hasExodusExtraBool = false;

    if (!isLevelGeo) {
        hasMtlName = (formatVer >= 9);
        hasSector = (formatVer >= 6);
        hasColGroup = (formatVer >= 7);
        hasRaycast = (formatVer >= 8);
        if (formatVer < 10) {
            hasPrimitive = false;
        }
        hasExodusExtraBool = (formatVer >= 11);
    } else {
        hasMtlName = (formatVer >= 14);
        hasSector = (formatVer >= 9);
        hasColGroup = (formatVer >= 10);
        hasRaycast = (formatVer >= 11);
        hasPrimitive = (formatVer >= 15);
        hasExodusExtraBool = (formatVer >= 16);
    }

    mShader = stream->ReadString();
    mTexList = stream->ReadString();
    mMaterialName1 = stream->ReadString();
    if (hasMtlName) {
        mMaterialName2 = stream->ReadString();
    }

    //mGameMaterial = GameMtlByTexture();

    if (hasSector) {
        mSector = stream->ReadWord();
    } else {
        mSector = 0xFFFF;
    }

    if (hasColGroup) {
        mCollisionGroup = stream->ReadWord();
    } else {
        mCollisionGroup = 0xFFFF;
    }

    if (hasRaycast) {
        mIsRaycast = stream->ReadBool();
    } else {
        mIsRaycast = true;
    }

    if (hasPrimitive) {
        mIsPrimitive = stream->ReadBool();
    } else {
        mIsPrimitive = false;
    }

    if (hasExodusExtraBool) {
        const bool extraBool = stream->ReadBool();
        // mIsExtraBool = (isLevelGeo && formatVer < 17) || extraBool;
    } else {
        // true
    }

    return true;
}

void MetroPhysicsCMesh::WriteMaterial(NxuBinaryStream* stream, const uint32_t formatVer, const bool isLevelGeo) const {
    //mIsDummy = stream->ReadBool();
    //if (mIsDummy) {
    //    mDummy = stream->ReadWord();
    //    return true;
    //}
    stream->WriteBool(false);

    bool hasMtlName = false;
    bool hasSector = false;
    bool hasColGroup = false;
    bool hasRaycast = false;
    bool hasPrimitive = true;
    bool hasExodusExtraBool = false;

    if (!isLevelGeo) {
        hasMtlName = (formatVer >= 9);
        hasSector = (formatVer >= 6);
        hasColGroup = (formatVer >= 7);
        hasRaycast = (formatVer >= 8);
        if (formatVer < 10) {
            hasPrimitive = false;
        }
        hasExodusExtraBool = (formatVer >= 11);
    } else {
        hasMtlName = (formatVer >= 14);
        hasSector = (formatVer >= 9);
        hasColGroup = (formatVer >= 10);
        hasRaycast = (formatVer >= 11);
        hasPrimitive = (formatVer >= 15);
        hasExodusExtraBool = (formatVer >= 16);
    }

    stream->WriteString(mShader);
    stream->WriteString(mTexList);
    stream->WriteString(mMaterialName1);
    if (hasMtlName) {
        stream->WriteString(mMaterialName2);
    }

    if (hasSector) {
        stream->WriteWord(mSector);
    }

    if (hasColGroup) {
        stream->WriteWord(mCollisionGroup);
    }

    if (hasRaycast) {
        stream->WriteBool(mIsRaycast);
    }

    if (hasPrimitive) {
        stream->WriteBool(mIsPrimitive);
    }

    if (hasExodusExtraBool) {
        stream->WriteBool(true);
    }
}
