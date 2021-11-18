#include "physx2utils.h"
#include "PhysXLoader.h"
#include "NxCooking.h"
#include "NxStream.h"
#include "NxTriangleMeshDesc.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


class NxuBinaryStream : public NxStream {
public:
    NxuBinaryStream() {}
    virtual ~NxuBinaryStream() {}

    void InitializeRead(const void* data, const size_t dataLength) {
        mReadStream = MemStream(data, dataLength);
    }

    void InitializeWrite() {
        // nothing here
    }

    // Loading API
    virtual NxU8 readByte()  const override {
        return mReadStream.ReadU8();
    }

    virtual NxU16 readWord() const override {
        return mReadStream.ReadU16();
    }

    virtual NxU32 readDword() const override {
        return mReadStream.ReadU32();
    }

    virtual NxF32 readFloat() const override {
        return mReadStream.ReadF32();
    }

    virtual NxF64 readDouble() const override {
        return mReadStream.ReadF64();
    }

    virtual void readBuffer(void* buffer, NxU32 size) const override {
        mReadStream.ReadToBuffer(buffer, size);
    }

    // Saving API
    virtual NxStream& storeByte(NxU8 b) override {
        mWriteStream.WriteU8(b);
        return *this;
    }

    virtual NxStream& storeWord(NxU16 w) override {
        mWriteStream.WriteU16(w);
        return *this;
    }

    virtual NxStream& storeDword(NxU32 d) override {
        mWriteStream.WriteU32(d);
        return *this;
    }

    virtual NxStream& storeFloat(NxF32 f) override {
        mWriteStream.WriteF32(f);
        return *this;
    }

    virtual NxStream& storeDouble(NxF64 f) override {
        mWriteStream.WriteF64(f);
        return *this;
    }

    virtual NxStream& storeBuffer(const void* buffer, NxU32 size) override {
        mWriteStream.Write(buffer, size);
        return *this;
    }

    void FlushToBytesArray(BytesArray& dst) {
        mWriteStream.SwapToBytesArray(dst);
    }

private:
    mutable MemStream   mReadStream;
    MemWriteStream      mWriteStream;
};



class PhysX2Utils final : public IPhysX2Utils {
public:
    PhysX2Utils();
    ~PhysX2Utils();

    bool            Initialize(const bool initCooker);

    virtual bool    CookedTriMeshToRawVertices(const void* cookedData, const size_t cookedDataSize, MyArray<vec3>& vertices, MyArray<uint16_t>& indices) override;
    virtual bool    RawVerticesToCookedTriMesh(const vec3* vertices, const size_t numVertices, const uint16_t* indices, const size_t numIndices, BytesArray& cookedData) override;

private:
    bool            InitializeSDK();
    bool            InitializeCooker();

private:
    NxPhysicsSDK*       mSDK;
    NxCookingInterface* mCooker;
};


namespace nux2 {
    bool CreatePhysX2Utils(const size_t initFlags, IPhysX2Utils** utils) {
        PhysX2Utils* createdUtils = new PhysX2Utils();
        if (!createdUtils->Initialize(0 != (initFlags & nux2::kInitCooker))) {
            MySafeDelete(createdUtils);
            return false;
        }
        utils[0] = createdUtils;
        return true;
    }

    void DestroyPhysX2Utils(IPhysX2Utils* utils) {
        if (utils) {
            PhysX2Utils* classUtils = scast<PhysX2Utils*>(utils);
            delete classUtils;
        }
    }
}



PhysX2Utils::PhysX2Utils()
    : mSDK(nullptr)
    , mCooker(nullptr)
{
}
PhysX2Utils::~PhysX2Utils() {
    if (mCooker) {
        mCooker->NxCloseCooking();
        mCooker = nullptr;
    }

    SetDllDirectoryW(nullptr);
}

bool PhysX2Utils::Initialize(const bool initCooker) {
    wchar_t sModulePath[1024] = { 0 };
    GetModuleFileNameW(GetModuleHandle(nullptr), sModulePath, 1024);

    fs::path dllsDir = sModulePath;
    dllsDir = dllsDir.parent_path() / L"physx/p2";
    SetDllDirectoryW(dllsDir.wstring().c_str());

    return this->InitializeSDK() && (!initCooker || this->InitializeCooker());
}

bool PhysX2Utils::CookedTriMeshToRawVertices(const void* cookedData, const size_t cookedDataSize, MyArray<vec3>& vertices, MyArray<uint16_t>& indices) {
    if (!mSDK) {
        return false;
    }

    NxuBinaryStream stream;
    stream.InitializeRead(cookedData, cookedDataSize);

    NxTriangleMesh* triMesh = mSDK->createTriangleMesh(stream);
    if (!triMesh) {
        return false;
    }

#define RELEASE_TRIMESH mSDK->releaseTriangleMesh(*triMesh); triMesh = nullptr;

    NxTriangleMeshDesc triMeshDesc;
    if (!triMesh->saveToDesc(triMeshDesc)) {
        RELEASE_TRIMESH
        return false;
    }

    if (!triMeshDesc.points || !triMeshDesc.triangles) {
        RELEASE_TRIMESH
        return false;
    }

    if (triMeshDesc.pointStrideBytes != sizeof(vec3)) { // if unsupported vertices format
        RELEASE_TRIMESH
        return false;
    }

    constexpr NxU32 kTriangles16BitSize = sizeof(uint16_t[3]);
    constexpr NxU32 kTriangles32BitSize = sizeof(uint32_t[3]);

    if (triMeshDesc.triangleStrideBytes != kTriangles16BitSize && triMeshDesc.triangleStrideBytes != kTriangles32BitSize) {   // if unsopported indices format
        RELEASE_TRIMESH
        return false;
    }

    const vec3* verticesData = rcast<const vec3*>(triMeshDesc.points);
    vertices.assign(verticesData, verticesData + triMeshDesc.numVertices);

    const size_t numIndices = triMeshDesc.numTriangles * 3;
    if (triMeshDesc.triangleStrideBytes == kTriangles16BitSize) {
        const uint16_t* indicesData = rcast<const uint16_t*>(triMeshDesc.triangles);
        indices.assign(indicesData, indicesData + numIndices);
    } else {
        const uint32_t* indicesData = rcast<const uint32_t*>(triMeshDesc.triangles);
        indices.resize(numIndices);
        for (size_t i = 0; i < numIndices; ++i) {
            indices[i] = scast<uint16_t>(indicesData[i]);
        }
    }

    return true;
}

bool PhysX2Utils::RawVerticesToCookedTriMesh(const vec3* vertices, const size_t numVertices, const uint16_t* indices, const size_t numIndices, BytesArray& cookedData) {
    if (!mCooker) {
        return false;
    }

    NxTriangleMeshDesc triMeshDesc;
    triMeshDesc.numVertices = scast<NxU32>(numVertices);
    triMeshDesc.numTriangles = scast<NxU32>(numIndices / 3);
    triMeshDesc.pointStrideBytes = sizeof(vec3);
    triMeshDesc.triangleStrideBytes = sizeof(uint16_t[3]);
    triMeshDesc.points = vertices;
    triMeshDesc.triangles = indices;
    triMeshDesc.flags = NX_MF_16_BIT_INDICES;

    NxuBinaryStream stream;
    stream.InitializeWrite();

    if (mCooker->NxCookTriangleMesh(triMeshDesc, stream)) {
        stream.FlushToBytesArray(cookedData);
        return true;
    } else {
        return false;
    }
}


bool PhysX2Utils::InitializeSDK() {
    if (mSDK) {
        return true;
    }

    NxPhysicsSDKDesc desc;
    NxSDKCreateError error;
    mSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, nullptr, nullptr, desc, &error);

    return mSDK != nullptr;
}

bool PhysX2Utils::InitializeCooker() {
    if (mCooker) {
        return true;
    }

    mCooker = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
    if (!mCooker) {
        return false;
    }

    if (!mCooker->NxInitCooking(nullptr, nullptr)) {
        mCooker = nullptr;
        return false;
    }

    return true;
}
