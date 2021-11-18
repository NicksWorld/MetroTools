#include "physx3utils.h"
#include "PxPhysicsAPI.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


class PxuInputStream : public physx::PxInputStream {
public:
    PxuInputStream() {}
    virtual ~PxuInputStream() {}

    void InitializeRead(const void* data, const size_t dataLength) {
        mStream = MemStream(data, dataLength);
    }

    virtual uint32_t read(void* dest, uint32_t count) override {
        const size_t cursorBefore = mStream.GetCursor();
        mStream.ReadToBuffer(dest, count);
        const size_t cursorAfter = mStream.GetCursor();
        return scast<uint32_t>(cursorAfter - cursorBefore);
    }

private:
    MemStream   mStream;
};

class PxuOutputStream : public physx::PxOutputStream {
public:
    PxuOutputStream() {}
    virtual ~PxuOutputStream() {}

    virtual uint32_t write(const void* src, uint32_t count) override {
        const size_t cursorBefore = mStream.GetWrittenBytesCount();
        mStream.Write(src, count);
        const size_t cursorAfter = mStream.GetWrittenBytesCount();
        return scast<uint32_t>(cursorAfter - cursorBefore);
    }

    void FlushToBytesArray(BytesArray& dst) {
        mStream.SwapToBytesArray(dst);
    }

private:
    MemWriteStream  mStream;
};

class PxuErrorCallback : public physx::PxErrorCallback {
public:
    PxuErrorCallback() {}
    virtual ~PxuErrorCallback() {}

    virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override {

    }
};


class PhysX3Utils final : public IPhysX3Utils {
public:
    PhysX3Utils();
    ~PhysX3Utils();

    bool            Initialize(const bool initCooker);

    virtual bool    CookedTriMeshToRawVertices(const void* cookedData, const size_t cookedDataSize, MyArray<vec3>& vertices, MyArray<uint16_t>& indices) override;
    virtual bool    RawVerticesToCookedTriMesh(const vec3* vertices, const size_t numVertices, const uint16_t* indices, const size_t numIndices, BytesArray& cookedData) override;

private:
    bool            InitializeSDK();
    bool            InitializeCooker();

private:
    physx::PxDefaultAllocator   mAllocator;
    PxuErrorCallback            mErrorCallback;
    physx::PxFoundation*        mFoundation;
    physx::PxPhysics*           mPhysics;
    physx::PxCooking*           mCooker;
};


namespace nux3 {
    bool CreatePhysX3Utils(const size_t initFlags, IPhysX3Utils** utils) {
        PhysX3Utils* createdUtils = new PhysX3Utils();
        if (!createdUtils->Initialize(0 != (initFlags & nux3::kInitCooker))) {
            MySafeDelete(createdUtils);
            return false;
        }
        utils[0] = createdUtils;
        return true;
    }

    void DestroyPhysX3Utils(IPhysX3Utils* utils) {
        if (utils) {
            PhysX3Utils* classUtils = scast<PhysX3Utils*>(utils);
            delete classUtils;
        }
    }
}




PhysX3Utils::PhysX3Utils()
    : mFoundation(nullptr)
    , mPhysics(nullptr)
    , mCooker(nullptr)
{
}
PhysX3Utils::~PhysX3Utils() {
    if (mCooker) {
        mCooker->release();
        mCooker = nullptr;
    }

    if (mPhysics) {
        mPhysics->release();
        mPhysics = nullptr;
    }

    if (mFoundation) {
        mFoundation->release();
        mFoundation = nullptr;
    }

    SetDllDirectoryW(nullptr);
}

bool PhysX3Utils::Initialize(const bool initCooker) {
    wchar_t sModulePath[1024] = { 0 };
    GetModuleFileNameW(GetModuleHandle(nullptr), sModulePath, 1024);

    fs::path dllsDir = sModulePath;
    dllsDir = dllsDir.parent_path() / L"physx/p3";
    SetDllDirectoryW(dllsDir.wstring().c_str());

    return this->InitializeSDK() && (!initCooker || this->InitializeCooker());
}

bool PhysX3Utils::CookedTriMeshToRawVertices(const void* cookedData, const size_t cookedDataSize, MyArray<vec3>& vertices, MyArray<uint16_t>& indices) {
    if (!mPhysics) {
        return false;
    }

    PxuInputStream stream;
    stream.InitializeRead(cookedData, cookedDataSize);

    physx::PxTriangleMesh* triMesh = mPhysics->createTriangleMesh(stream);
    if (!triMesh) {
        return false;
    }

#define RELEASE_TRIMESH triMesh->release(); triMesh = nullptr;

    const size_t numTriangles = triMesh->getNbTriangles();
    const size_t numVertices = triMesh->getNbVertices();

    const void* ptrTriangles = triMesh->getTriangles();
    const vec3* ptrVertices = rcast<const vec3*>(triMesh->getVertices());

    if (!numVertices || !numTriangles || !ptrTriangles || !ptrVertices) {
        RELEASE_TRIMESH
        return false;
    }

    const bool isIndices16Bit = triMesh->getTriangleMeshFlags().isSet(physx::PxTriangleMeshFlag::e16_BIT_INDICES);


    vertices.assign(ptrVertices, ptrVertices + numVertices);

    const size_t numIndices = numTriangles * 3;
    if (isIndices16Bit) {
        const uint16_t* indicesData = rcast<const uint16_t*>(ptrTriangles);
        indices.assign(indicesData, indicesData + numIndices);
    } else {
        const uint32_t* indicesData = rcast<const uint32_t*>(ptrTriangles);
        indices.resize(numIndices);
        for (size_t i = 0; i < numIndices; ++i) {
            indices[i] = scast<uint16_t>(indicesData[i]);
        }
    }

    return true;
}

bool PhysX3Utils::RawVerticesToCookedTriMesh(const vec3* vertices, const size_t numVertices, const uint16_t* indices, const size_t numIndices, BytesArray& cookedData) {
    if (!mCooker) {
        return false;
    }

    physx::PxTriangleMeshDesc triMeshDesc;
    triMeshDesc.points.count = scast<physx::PxU32>(numVertices);
    triMeshDesc.points.stride = sizeof(vec3);
    triMeshDesc.points.data = vertices;
    triMeshDesc.triangles.count = scast<physx::PxU32>(numIndices / 3);
    triMeshDesc.triangles.stride = sizeof(uint16_t[3]);
    triMeshDesc.triangles.data = indices;
    triMeshDesc.flags = physx::PxMeshFlag::e16_BIT_INDICES;

    PxuOutputStream stream;
    if (mCooker->cookTriangleMesh(triMeshDesc, stream)) {
        stream.FlushToBytesArray(cookedData);
        return true;
    } else {
        return false;
    }
}


bool PhysX3Utils::InitializeSDK() {
    if (mFoundation && mPhysics) {
        return true;
    }

    mFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, mAllocator, mErrorCallback);
    if (!mFoundation) {
        return false;
    }

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale());

    return mPhysics != nullptr;
}

bool PhysX3Utils::InitializeCooker() {
    if (mCooker) {
        return true;
    }

    mCooker = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, physx::PxCookingParams(physx::PxTolerancesScale()));

    return mCooker != nullptr;
}
