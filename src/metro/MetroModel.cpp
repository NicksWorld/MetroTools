#include "MetroModel.h"
#include "MetroContext.h"
#include "MetroSkeleton.h"
#include "MetroMotion.h"


enum ModelChunks {
    MC_HeaderChunk          = 0x00000001,
    MC_MaterialsChunk       = 0x00000002,
    MC_VerticesChunk        = 0x00000003,
    MC_FacesChunk           = 0x00000004,
    MC_SkinnedVerticesChunk = 0x00000005,

    MC_ChildrenChunk        = 0x00000009,
    MC_ChildrenRefsChunk    = 0x0000000A,   // 10

    MC_Lod_1_Chunk          = 0x0000000B,   // 11
    MC_Lod_2_Chunk          = 0x0000000C,   // 12

    MC_MeshesInline         = 0x0000000F,   // 15
    MC_MeshesLinks          = 0x00000010,   // 16

    MC_SkeletonLink         = 0x00000014,   // 20

    MC_MeshRef              = 0x00000015,   // 21

    MC_SkeletonInline       = 0x00000018,   // 24

    MC_TexturesReplacements = 0x0000001D,   // 29

    MC_TexturesPresets      = 0x00000020,   // 32

    MC_Comment              = 0x00000024,   // 36
};


static const size_t kModelVersionLastLight          = 17; // 18 spotted in early XBox builds
static const size_t kModelVersionLastLightRelease   = 20; // 20/21 seems release version
static const size_t kModelVersionRedux              = 22;
static const size_t kModelVersionEarlyArktika1      = 30;
static const size_t kModelVersionArktika1           = 31;
static const size_t kModelVersionExodus             = 42;

static const size_t kMetroModelMaxMaterials = 4;

PACKED_STRUCT_BEGIN
struct MdlHeader {          // size = 64
    enum : uint32_t {
        Flag_ModelIsDraft = 1
    };

    uint8_t     version;
    uint8_t     type;
    uint16_t    shaderId;
    AABBox      bbox;
    BSphere     bsphere;
    uint32_t    checkSum;
    float       invLod;
    uint32_t    flags;
    float       vscale;
    float       texelDensity;
} PACKED_STRUCT_END;




// Base class for all Metro models
MetroModelBase::MetroModelBase()
    : mVersion(0)
    , mType(0)
    , mFlags(0)
    , mEngineMtl(0xFFFF)
    , mChecksum(0)
    , mSSABias(0.0f)
    , mMaterialFlags0(2)
    , mMaterialFlags1(4)
    , mIsCollisionModel(false)
{
}
MetroModelBase::~MetroModelBase() {
}

bool MetroModelBase::Load(MemStream& stream, MetroModelLoadParams& params) {
    bool headerFound = false;

    StreamChunker chunker(stream);

    MdlHeader hdr = {};

    MemStream headerStream = chunker.GetChunkStream(MC_HeaderChunk);
    if (headerStream && headerStream.Remains() >= sizeof(MdlHeader)) {
        headerStream.ReadStruct(hdr);

        mVersion = hdr.version;
        mType = hdr.type;
        mFlags = hdr.flags;
        mEngineMtl = hdr.shaderId;
        mChecksum = hdr.checkSum;

        mBBox.minimum = MetroSwizzle(hdr.bbox.minimum);
        mBBox.maximum = MetroSwizzle(hdr.bbox.maximum);
        mBSphere.center = MetroSwizzle(hdr.bsphere.center);
        mBSphere.radius = hdr.bsphere.radius;

        if (headerStream.Remains() >= sizeof(float)) {
            mSSABias = headerStream.ReadF32();
        }

        headerFound = true;
    }

    MemStream materialsStream = chunker.GetChunkStream(MC_MaterialsChunk);
    if (materialsStream) {
        mMaterialStrings.resize(4);

        mMaterialStrings[0] = materialsStream.ReadStringZ();    // texture name
        mMaterialStrings[1] = materialsStream.ReadStringZ();    // shader name
        mMaterialStrings[2] = materialsStream.ReadStringZ();    // game material name
        if (params.formatVersion >= kModelVersionLastLight) {
            mMaterialStrings[3] = materialsStream.ReadStringZ();

            //#NOTE_SK: here's the game, depending on some other flags, might read uint16_t
            //          uint16_t matFlags = materialsStream.ReadU16();
            //          bool ignore_model = matFlags & 8;
            mMaterialFlags0 = materialsStream.ReadU16();
            mMaterialFlags1 = materialsStream.ReadU16();

            if (StrContains(mMaterialStrings[3], "collision")) {
                mIsCollisionModel = true;
            }
        }
    }

    if (mMesh) {
        float vscale;
        if (hdr.version <= kModelVersionEarlyArktika1) {
            if (hdr.version >= 29) { // 29 and 30
                const vec3 t = mBBox.maximum - mBBox.minimum;
                vscale = Max3(fabsf(t.x), fabsf(t.y), fabsf(t.z));
            } else {
                vscale = 12.0f;
            }
        } else {
            vscale = hdr.vscale;
        }

        mMesh->verticesScale = vscale;
    }

    return headerFound;
}

bool MetroModelBase::Save(MemWriteStream& stream) {
    // header
    {
        ChunkWriteHelper headerChunk(stream, MC_HeaderChunk);
        MdlHeader hdr = {};
        hdr.version = scast<uint8_t>(mVersion);
        hdr.type = scast<uint8_t>(mType);
        hdr.shaderId = scast<uint16_t>(mEngineMtl);
        hdr.bbox.minimum = MetroSwizzle(mBBox.minimum);
        hdr.bbox.maximum = MetroSwizzle(mBBox.maximum);
        hdr.bsphere.center = MetroSwizzle(mBSphere.center);
        hdr.bsphere.radius = mBSphere.radius;
        stream.Write(hdr);
    }

    // materials
    if(!mMaterialStrings.empty())
    {
        ChunkWriteHelper materialsChunk(stream, MC_MaterialsChunk);
        stream.WriteStringZ(mMaterialStrings[0]);
        stream.WriteStringZ(mMaterialStrings[1]);
        stream.WriteStringZ(mMaterialStrings[2]);

        if (mVersion >= kModelVersionLastLight) {
            stream.WriteStringZ(mMaterialStrings[3]);

            stream.WriteU16(mMaterialFlags0);
            stream.WriteU16(mMaterialFlags1);
        }
    }

    return true;
}

void MetroModelBase::SetSourceName(const CharString& srcName) {
    mSourceName = srcName;
}

const CharString& MetroModelBase::GetSourceName() const {
    return mSourceName;
}

MetroModelType MetroModelBase::GetModelType() const {
    return scast<MetroModelType>(mType);
}

size_t MetroModelBase::GetModelVersion() const {
    return mVersion;
}

void MetroModelBase::SetModelVersion(const size_t version) {
    mVersion = scast<uint16_t>(version);
}

uint32_t MetroModelBase::GetCheckSum() const {
    return mChecksum;
}

size_t MetroModelBase::GetLodCount() const {
    return 0;
}

const AABBox& MetroModelBase::GetBBox() const {
    return mBBox;
}

const BSphere& MetroModelBase::GetBSphere() const {
    return mBSphere;
}

const CharString& MetroModelBase::GetMaterialString(const size_t idx) const {
    return mMaterialStrings[idx];
}

void MetroModelBase::SetMaterialString(const CharString& str, const size_t idx) {
    if (mMaterialStrings.empty()) {
        mMaterialStrings.resize(4);
    }

    if (idx < mMaterialStrings.size()) {
        mMaterialStrings[idx] = str;
    }
}

bool MetroModelBase::IsCollisionModel() const {
    return mIsCollisionModel;
}

bool MetroModelBase::MeshValid() const {
    return mMesh != nullptr;
}

RefPtr<MetroModelMesh> MetroModelBase::GetMesh() const {
    return mMesh;
}

size_t MetroModelBase::GetVerticesCount() const {
    return mMesh ? mMesh->verticesCount : 0;
}

size_t MetroModelBase::GetFacesCount() const {
    return mMesh ? mMesh->facesCount : 0;
}

uint32_t MetroModelBase::GetVertexType() const {
    return mMesh ? mMesh->vertexType : 0;
}

void MetroModelBase::CollectGeomData(MyArray<MetroModelGeomData>& result, const size_t) const {
    if (this->MeshValid()) {
        MetroModelGeomData gd = {
            mBBox,
            mBSphere,
            mMaterialStrings.empty() ? kEmptyString : mMaterialStrings.front(),
            mMesh.get(),
            this,
            this->GetVerticesMemData(),
            this->GetFacesMemData()
        };

        result.push_back(gd);
    }
}



// Simple static model, could be just a *.mesh file
MetroModelStd::MetroModelStd()
    : Base()
{
    mType = scast<uint16_t>(MetroModelType::Std);
}
MetroModelStd::~MetroModelStd() {
}

bool MetroModelStd::Load(MemStream& stream, MetroModelLoadParams& params) {
    StreamChunker chunker(stream);

    // load mesh
    MemStream meshRefStream = chunker.GetChunkStream(MC_MeshRef);
    if (meshRefStream) {
        uint32_t vtxType = meshRefStream.ReadU32();
        if (MetroVertexType::LevelLegacy == vtxType) {
            // "! level's mesh contains deprecated vertex format! Use level compiler to update!"
            vtxType = MetroVertexType::Level;
        }

        mMesh = MakeRefPtr<MetroModelMesh>();

        mMesh->verticesOffset = meshRefStream.ReadU32();
        mMesh->verticesCount = meshRefStream.ReadU32();

        mMesh->indicesOffset = meshRefStream.ReadU32();
        mMesh->facesCount = meshRefStream.ReadU32() / 3;

        mMesh->vertexType = vtxType;
    } else {
        mMesh = MakeRefPtr<MetroModelMesh>();

        // vertices
        MemStream verticesStream = chunker.GetChunkStream(MC_VerticesChunk);
        assert(verticesStream);

        mMesh->vertexType = verticesStream.ReadU32();
        mMesh->verticesCount = verticesStream.ReadU32();
        if (params.formatVersion >= kModelVersionEarlyArktika1) {
            mMesh->shadowVerticesCount = verticesStream.ReadU16();
        }

        if (TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadGeometry)) {
            mVerticesData.resize(verticesStream.Remains());
            verticesStream.ReadToBuffer(mVerticesData.data(), mVerticesData.size());
        }

        // faces
        MemStream facesStream = chunker.GetChunkStream(MC_FacesChunk);
        assert(facesStream);

        if (params.formatVersion < kModelVersionEarlyArktika1) {
            mMesh->facesCount = facesStream.ReadU32() / 3;
        } else {
            mMesh->facesCount = facesStream.ReadU32();
            mMesh->shadowFacesCount = facesStream.ReadU16();
        }

        if (TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadGeometry)) {
            mFacesData.resize(facesStream.Remains());
            facesStream.ReadToBuffer(mFacesData.data(), mFacesData.size());
        }
    }

    // load base
    const bool baseLoaded = MetroModelBase::Load(stream, params);

    return mMesh && baseLoaded;
}

bool MetroModelStd::Save(MemWriteStream& stream) {
    bool result = false;

    if (this->MeshValid()) {
        Base::Save(stream);

        // vertices
        {
            ChunkWriteHelper verticesChunk(stream, MC_VerticesChunk);

            stream.WriteU32(mMesh->vertexType);
            stream.WriteU32(mMesh->verticesCount);
            if (mVersion >= kModelVersionEarlyArktika1) {
                stream.WriteU16(scast<uint16_t>(mMesh->shadowVerticesCount));
            }

            stream.Write(mVerticesData.data(), mVerticesData.size());
        }

        // faces
        {
            ChunkWriteHelper facesChunk(stream, MC_FacesChunk);

            if (mVersion < kModelVersionEarlyArktika1) {
                stream.WriteU32(mMesh->facesCount * 3);
            } else {
                stream.WriteU32(mMesh->facesCount);
                stream.WriteU16(scast<uint16_t>(mMesh->shadowFacesCount));
            }
            stream.Write(mFacesData.data(), mFacesData.size());
        }

        result = true;
    }

    return result;
}

size_t MetroModelStd::GetVerticesMemSize() const {
    return mVerticesData.size();
}

const void* MetroModelStd::GetVerticesMemData() const {
    return mVerticesData.data();
}

size_t MetroModelStd::GetFacesMemSize() const {
    return mFacesData.size();
}

const void* MetroModelStd::GetFacesMemData() const {
    return mFacesData.data();
}

void MetroModelStd::FreeGeometryMem() {
    mVerticesData.clear();
    mFacesData.clear();
}

// model creation
void MetroModelStd::CreateMesh(const size_t numVertices, const size_t numFaces) {
    mMesh = MakeRefPtr<MetroModelMesh>();
    memset(mMesh.get(), 0, sizeof(MetroModelMesh));

    mMesh->verticesCount = scast<uint32_t>(numVertices);
    mMesh->facesCount = scast<uint32_t>(numFaces);
    mMesh->vertexType = MetroVertexType::Static;
    mMesh->verticesScale = 1.0f;

    mVerticesData.resize(numVertices * sizeof(VertexStatic));
    mFacesData.resize(numFaces * sizeof(MetroFace));
}

void MetroModelStd::CopyVerticesData(const void* vertices) {
    memcpy(mVerticesData.data(), vertices, mVerticesData.size());
}

void MetroModelStd::CopyFacesData(const void* faces) {
    memcpy(mFacesData.data(), faces, mFacesData.size());
}

void MetroModelStd::SetBounds(const AABBox& bbox, const BSphere& bsphere) {
    mBBox = bbox;
    mBSphere = bsphere;
}


// Simple skinned model, could be just a *.mesh file
MetroModelSkin::MetroModelSkin()
    : Base()
    , mParent(nullptr)
{
    mType = scast<uint16_t>(MetroModelType::Skin);
}
MetroModelSkin::~MetroModelSkin() {
}

bool MetroModelSkin::Load(MemStream& stream, MetroModelLoadParams& params) {
    StreamChunker chunker(stream);

    // load mesh

    // vertices
    MemStream verticesStream = chunker.GetChunkStream(MC_SkinnedVerticesChunk);
    assert(verticesStream);

    mMesh = MakeRefPtr<MetroModelMesh>();

    const size_t numBones = verticesStream.ReadU8();

    mMesh->bonesRemap.resize(numBones);
    verticesStream.ReadToBuffer(mMesh->bonesRemap.data(), numBones);

    mBonesOBB.resize(numBones);
    verticesStream.ReadToBuffer(mBonesOBB.data(), numBones * sizeof(MetroOBB));

    mMesh->verticesCount = verticesStream.ReadU32();
    if (params.formatVersion >= kModelVersionEarlyArktika1) {
        mMesh->shadowVerticesCount = verticesStream.ReadU16();
    }
    mMesh->vertexType = MetroVertexType::Skin;

    if (TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadGeometry)) {
        mVerticesData.resize(verticesStream.Remains());
        verticesStream.ReadToBuffer(mVerticesData.data(), mVerticesData.size());
    }


    // faces
    MemStream facesStream = chunker.GetChunkStream(MC_FacesChunk);
    assert(facesStream);

    if (params.formatVersion < kModelVersionLastLight) {
        mMesh->facesCount = facesStream.ReadU32() / 3;
    } else {
        mMesh->facesCount = facesStream.ReadU16();
        mMesh->shadowFacesCount = facesStream.ReadU16();
    }

    if (TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadGeometry)) {
        mFacesData.resize(facesStream.Remains());
        facesStream.ReadToBuffer(mFacesData.data(), mFacesData.size());
    }


    // load base
    const bool baseLoaded = MetroModelBase::Load(stream, params);

    return mMesh && baseLoaded;
}

bool MetroModelSkin::Save(MemWriteStream& stream) {
    bool result = false;

    if (this->MeshValid()) {
        Base::Save(stream);

        // vertices
        {
            ChunkWriteHelper verticesChunk(stream, MC_SkinnedVerticesChunk);

            stream.WriteU8(scast<uint8_t>(mMesh->bonesRemap.size() & 0xFF));
            stream.Write(mMesh->bonesRemap.data(), mMesh->bonesRemap.size());
            stream.Write(mBonesOBB.data(), mBonesOBB.size() * sizeof(MetroOBB));

            stream.WriteU32(mMesh->verticesCount);
            if (mVersion >= kModelVersionEarlyArktika1) {
                stream.WriteU16(scast<uint16_t>(mMesh->shadowVerticesCount));
            }

            stream.Write(mVerticesData.data(), mVerticesData.size());
        }

        // faces
        {
            ChunkWriteHelper facesChunk(stream, MC_FacesChunk);

            if (mVersion < kModelVersionLastLight) {
                stream.WriteU32(mMesh->facesCount * 3);
            } else {
                stream.WriteU16(scast<uint16_t>(mMesh->facesCount));
                stream.WriteU16(scast<uint16_t>(mMesh->shadowFacesCount));
            }

            stream.Write(mFacesData.data(), mFacesData.size());
        }
    }

    return result;
}

size_t MetroModelSkin::GetVerticesMemSize() const {
    return mVerticesData.size();
}

const void* MetroModelSkin::GetVerticesMemData() const {
    return mVerticesData.data();
}

size_t MetroModelSkin::GetFacesMemSize() const {
    return mFacesData.size();
}

const void* MetroModelSkin::GetFacesMemData() const {
    return mFacesData.data();
}

void MetroModelSkin::FreeGeometryMem() {
    mVerticesData.clear();
    mFacesData.clear();
}

MetroModelSkeleton* MetroModelSkin::GetParent() const {
    return mParent;
}

void MetroModelSkin::SetParent(MetroModelSkeleton* parent) {
    mParent = parent;
}



// Complete static model, can consist of multiple Std models (inline or external *.mesh files)
MetroModelHierarchy::MetroModelHierarchy()
    : Base()
{
    mType = scast<uint16_t>(MetroModelType::Hierarchy);

    mBBox.Reset();
    mBSphere.Reset();
}
MetroModelHierarchy::~MetroModelHierarchy() {
}

bool MetroModelHierarchy::Load(MemStream& stream, MetroModelLoadParams& params) {
    bool result = false;

    StreamChunker chunker(stream);

    if (MetroModelBase::Load(stream, params)) {
        MemStream childrenRefsStream = chunker.GetChunkStream(MC_ChildrenRefsChunk);
        if (childrenRefsStream) {
            //#TODO_SK: implement fully!
            const size_t meshesCount = childrenRefsStream.ReadU32();
            mChildren.reserve(meshesCount);
            for (size_t i = 0; i < meshesCount; ++i) {
                const size_t meshIdx = childrenRefsStream.ReadU32();
                // ?????
                //mChildren.push_back(somewhere[meshIdx]);
            }
        } else {
            MemStream childrenStream = chunker.GetChunkStream(MC_ChildrenChunk);
            if (childrenStream) {
                StreamChunker childrenChunks(childrenStream);
                const size_t childrenChunksCount = childrenChunks.GetChunksCount();

                result = true;

                mChildren.reserve(childrenChunksCount);
                for (size_t i = 0; i < childrenChunksCount; ++i) {
                    const size_t childChunkId = childrenChunks.GetChunkIDByIdx(i);
                    if (childChunkId == i) {
                        MemStream childStream = childrenChunks.GetChunkStreamByIdx(i);
                        RefPtr<MetroModelBase> child = MetroModelFactory::CreateModelFromStream(childStream, params.loadFlags);
                        if (child) {
                            bool skipChild = false;
                            if (!TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadCollision) && child->IsCollisionModel()) {
                                skipChild = true;
                            }

                            if (!skipChild) {
                                this->AddChild(child);
                            }
                        } else {
                            result = false;
                            break;
                        }
                    }
                }
            }

            MemStream lod1Stream = chunker.GetChunkStream(MC_Lod_1_Chunk);
            RefPtr<MetroModelBase> lod1Model = lod1Stream ? MetroModelFactory::CreateModelFromStream(lod1Stream, params.loadFlags) : nullptr;

            MemStream lod2Stream = chunker.GetChunkStream(MC_Lod_2_Chunk);
            RefPtr<MetroModelBase> lod2Model = lod2Stream ? MetroModelFactory::CreateModelFromStream(lod2Stream, params.loadFlags) : nullptr;

            if (lod1Model) {
                mLods.push_back(lod1Model);
            }
            if (lod2Model) {
                mLods.push_back(lod2Model);
            }
        }
    }

    return result;
}

bool MetroModelHierarchy::Save(MemWriteStream& stream) {
    Base::Save(stream);

    // children
    {
        ChunkWriteHelper childrenChunk(stream, MC_ChildrenChunk);

        const size_t numChildren = mChildren.size();
        for (size_t i = 0; i < numChildren; ++i) {
            ChunkWriteHelper childChunk(stream, i);
            mChildren[i]->Save(stream);
        }
    }

    // LOD 1
    if (mLods.size() > 0) {
        ChunkWriteHelper lod1Chunk(stream, MC_Lod_1_Chunk);
        mLods[0]->Save(stream);
    }

    // LOD 2
    if (mLods.size() > 1) {
        ChunkWriteHelper lod2Chunk(stream, MC_Lod_2_Chunk);
        mLods[1]->Save(stream);
    }

    return true;
}

size_t MetroModelHierarchy::GetLodCount() const {
    return mLods.size();
}

void MetroModelHierarchy::FreeGeometryMem() {
    for (auto& child : mChildren) {
        child->FreeGeometryMem();
    }

    for (auto& lod : mLods) {
        lod->FreeGeometryMem();
    }
}

void MetroModelHierarchy::CollectGeomData(MyArray<MetroModelGeomData>& result, const size_t lodIdx) const {
    if (kInvalidValue == lodIdx) {
        for (auto& child : mChildren) {
            child->CollectGeomData(result);
        }
    } else if (lodIdx < mLods.size()) {
        mLods[lodIdx]->CollectGeomData(result);
    }
}

size_t MetroModelHierarchy::GetChildrenCount() const {
    return mChildren.size();
}

RefPtr<MetroModelBase> MetroModelHierarchy::GetChild(const size_t idx) const {
    return mChildren[idx];
}

void MetroModelHierarchy::AddChild(const RefPtr<MetroModelBase>& child) {
    mChildren.emplace_back(child);

    if (!mBSphere.Valid()) {
        mBSphere = child->GetBSphere();
        mBBox = child->GetBBox();
    } else {
        mBSphere.Absorb(child->GetBSphere());
        mBBox.Absorb(child->GetBBox());
    }
}


// Complete animated model, can consist of multiple Skin models (inline or external *.mesh files)
MetroModelSkeleton::MetroModelSkeleton()
    : Base()
{
    mType = scast<uint16_t>(MetroModelType::Skeleton);
}
MetroModelSkeleton::~MetroModelSkeleton() {
}

bool MetroModelSkeleton::Load(MemStream& stream, MetroModelLoadParams& params) {
    bool result = true;

    StreamChunker chunker(stream);

    const bool hasHeader = MetroModelBase::Load(stream, params);

    MemStream meshesLinksStream = chunker.GetChunkStream(MC_MeshesLinks);
    if (meshesLinksStream) {
        const size_t numStrings = meshesLinksStream.ReadU32();  // not used ???
        mLodMeshes.resize(3);
        for (size_t i = 0; i < 3; ++i) {
            CharString meshesNames = meshesLinksStream.ReadStringZ();
            if (!i) {
                if (!this->LoadLodMeshes(this, meshesNames, params, i)) {
                    result = false;
                    break;
                }
            } else {
                RefPtr<MetroModelHierarchy> lodModel = SCastRefPtr<MetroModelHierarchy>(MetroModelFactory::CreateModelFromType(MetroModelType::Hierarchy2));
                if (this->LoadLodMeshes(lodModel.get(), meshesNames, params, i)) {
                    mLods.push_back(lodModel);
                }
            }
        }
    } else {
        MemStream meshesStream = chunker.GetChunkStream(MC_MeshesInline);
        if (meshesStream) {
            mLodMeshes.resize(3);
            StreamChunker lodsChunks(meshesStream);
            for (size_t i = 0; i < 3 && i < lodsChunks.GetChunksCount(); ++i) {
                if (lodsChunks.GetChunkIDByIdx(i) == i) {
                    MemStream lodStream = lodsChunks.GetChunkStreamByIdx(i);
                    if (!i) {
                        if (!this->LoadLodMeshes(this, lodStream, params, i)) {
                            result = false;
                            break;
                        }
                    } else {
                        RefPtr<MetroModelHierarchy> lodModel = SCastRefPtr<MetroModelHierarchy>(MetroModelFactory::CreateModelFromType(MetroModelType::Hierarchy2));
                        if (this->LoadLodMeshes(lodModel.get(), lodStream, params, i)) {
                            mLods.push_back(lodModel);
                        }
                    }
                }
            }
        }
    }

    if (TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadSkeleton)) {
        MemStream skeletonLinkStream = chunker.GetChunkStream(MC_SkeletonLink);
        if (skeletonLinkStream) {
            CharString skeletonRef = skeletonLinkStream.ReadStringZ();
            CharString fullSkelPath = MetroFileSystem::Paths::MeshesFolder + skeletonRef + MetroContext::Get().GetSkeletonExtension();
            MemStream skeletonStream = MetroContext::Get().GetFilesystem().OpenFileFromPath(fullSkelPath);
            if (skeletonStream) {
                mSkeleton = MakeRefPtr<MetroSkeleton>();
                if (!mSkeleton->LoadFromData(skeletonStream)) {
                    mSkeleton = nullptr;
                }
            }
        } else {
            MemStream skeletonStream = chunker.GetChunkStream(MC_SkeletonInline);
            if (skeletonStream) {
                mSkeleton = MakeRefPtr<MetroSkeleton>();
                if (!mSkeleton->LoadFromData(skeletonStream)) {
                    mSkeleton = nullptr;
                }
            }
        }
    }

    return result;
}

bool MetroModelSkeleton::Save(MemWriteStream& stream) {
    // !!! Here we don't want to run Save function from Hierarchy class, just BaseModel class to write the header
    MetroModelBase::Save(stream);

    // meshes
    {
        ChunkWriteHelper meshesChunk(stream, MC_MeshesInline);

        const size_t numLods = mLods.size() + 1;
        for (size_t i = 0; i < numLods; ++i) {
            ChunkWriteHelper lodChunk(stream, i);

            const size_t numLodMeshes = mLodMeshes[i].size();
            if (numLodMeshes) {
                for (size_t j = 0; j < numLodMeshes; ++j) {
                    ChunkWriteHelper lodMeshChunk(stream, j);

                    ModelPtr& lodMeshPtr = mLodMeshes[i][j];
                    lodMeshPtr->Save(stream);
                }
            }
        }
    }

    // skeleton
    if (mSkeleton) {
        //#TODO_SK: implement skeleton saving!
        //mSkeleton->
    }

    return true;
}

size_t MetroModelSkeleton::GetLodCount() const {
    return 0;
}

const RefPtr<MetroSkeleton>& MetroModelSkeleton::GetSkeleton() const {
    return mSkeleton;
}

bool MetroModelSkeleton::LoadLodMeshes(MetroModelHierarchy* target, CharString& meshesNames, MetroModelLoadParams& params, const size_t lodIdx) {
    bool result = true;

    MyArray<StringView> names = StrSplitViews(meshesNames, ',');
    if (!names.empty()) {
        for (StringView n : names) {
            MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

            MetroFSPath file(MetroFSPath::Invalid);
            if (n[0] == '.' && n[1] == kPathSeparator) { // relative path
                MetroFSPath folder = mfs.GetParentFolder(params.srcFile);
                file = mfs.FindFile(CharString(n.substr(2)) + ".mesh", folder);
            } else {
                CharString meshFilePath = CharString(MetroFileSystem::Paths::MeshesFolder).append(n).append(".mesh");
                file = mfs.FindFile(meshFilePath);
            }

            if (file.IsValid()) {
                MemStream stream = mfs.OpenFileStream(file);
                stream.SetName(mfs.GetName(file));
                if (!this->LoadLodMesh(target, stream, params, lodIdx)) {
                    result = false;
                    break;
                }
            } else {
                assert(false);
            }
        }
    } else {
        result = false;
    }

    return result;
}

bool MetroModelSkeleton::LoadLodMeshes(MetroModelHierarchy* target, MemStream& meshesStream, MetroModelLoadParams& params, const size_t lodIdx) {
    bool result = true;

    StreamChunker chunker(meshesStream);
    if (chunker.GetChunksCount() > 0) {
        for (size_t i = 0; i < chunker.GetChunksCount(); ++i) {
            if (chunker.GetChunkIDByIdx(i) != i) {
                result = false;
                break;
            }

            MemStream stream = chunker.GetChunkStreamByIdx(i);
            if (!this->LoadLodMesh(target, stream, params, lodIdx)) {
                result = false;
                break;
            }
        }
    } else {
        result = false;
    }

    return result;
}

bool MetroModelSkeleton::LoadLodMesh(MetroModelHierarchy* target, MemStream& stream, MetroModelLoadParams& params, const size_t lodIdx) {
    bool result = false;

    if (stream) {
        RefPtr<MetroModelBase> mesh = MetroModelFactory::CreateModelFromStream(stream, params.loadFlags);
        if (mesh) {
            //#NOTE_SK: are we sure it's always hierarchy/skeleton ?
            RefPtr<MetroModelHierarchy> hm = SCastRefPtr<MetroModelHierarchy>(mesh);
            const size_t childrenCount = hm->GetChildrenCount();
            for (size_t i = 0; i < childrenCount; ++i) {
                //#NOTE_SK: are we sure it's always skin mesh ?
                RefPtr<MetroModelSkin> child = SCastRefPtr<MetroModelSkin>(hm->GetChild(i));
                target->AddChild(child);
                child->SetParent(this);
            }

            mLodMeshes[lodIdx].push_back(mesh);

            result = true;
        }
    }

    return result;
}




RefPtr<MetroModelBase> MetroModelFactory::CreateModelFromType(const MetroModelType type) {
    RefPtr<MetroModelBase> result;

    switch (type) {
        case MetroModelType::Std: {
            result = MakeRefPtr<MetroModelStd>();
        } break;

        case MetroModelType::Hierarchy:
        case MetroModelType::Hierarchy2: {
            result = MakeRefPtr<MetroModelHierarchy>();
        } break;

        case MetroModelType::Skeleton:
        case MetroModelType::Skeleton2:
        case MetroModelType::Skeleton3: {
            result = MakeRefPtr<MetroModelSkeleton>();
        } break;

        case MetroModelType::Skin: {
            result = MakeRefPtr<MetroModelSkin>();
        } break;

        case MetroModelType::Soft: {
            //#TODO_SK: Implement!
            assert(false && "Implement me!!!");
        } break;

        case MetroModelType::ParticlesEffect: {
            //#TODO_SK: Implement!
            assert(false && "Implement me!!!");
        } break;

        case MetroModelType::ParticlesSystem: {
            //#TODO_SK: Implement!
            assert(false && "Implement me!!!");
        } break;
    }

    return result;
}

RefPtr<MetroModelBase> MetroModelFactory::CreateModelFromStream(MemStream& stream, const uint32_t loadFlags, const MetroFSPath& srcFile) {
    RefPtr<MetroModelBase> result;

    StreamChunker chunker(stream);

    MemStream headerStream = chunker.GetChunkStream(MC_HeaderChunk);
    if (headerStream) {
        MdlHeader hdr;
        headerStream.ReadStruct(hdr);

        result = MetroModelFactory::CreateModelFromType(scast<MetroModelType>(hdr.type));
        if (result) {
            MetroModelLoadParams params = {
                kEmptyString,
                kEmptyString,
                hdr.version,
                loadFlags,
                srcFile
            };

            const bool success = result->Load(stream, params);
            if (!success) {
                result = nullptr;
            } else {
                result->SetSourceName(stream.Name());
            }
        }
    }

    return result;
}

RefPtr<MetroModelBase> MetroModelFactory::CreateModelFromFile(const MetroFSPath& file, const uint32_t loadFlags) {
    MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(file);
    if (stream) {
        return MetroModelFactory::CreateModelFromStream(stream, loadFlags, file);
    } else {
        return nullptr;
    }
}




MetroModel::MetroModel()
    : mType(0)
    , mSkeleton(nullptr)
    , mCurrentMesh(nullptr)
    , mThisFileIdx(MetroFSPath::Invalid)
{
    for (size_t i = 0; i < kMetroModelMaxLods; ++i) {
        mLodModels[i] = nullptr;
    }
}
MetroModel::~MetroModel() {
    std::for_each(mMeshes.begin(), mMeshes.end(), [](MetroMesh* mesh) { MySafeDelete(mesh); });
    std::for_each(mMotions.begin(), mMotions.end(), [](MetroModel::MotionInfo& mi) { MySafeDelete(mi.motion); });
    MySafeDelete(mSkeleton);
    for (size_t i = 0; i < kMetroModelMaxLods; ++i) {
        MySafeDelete(mLodModels[i]);
    }
}

bool MetroModel::LoadFromName(const CharString& name, const bool needAnimations) {
    bool result = false;

    CharString modelPath = name, tpreset;
    const size_t atPos = name.find('@');
    if (atPos != CharString::npos) {
        modelPath = name.substr(0, atPos);
        tpreset = name.substr(atPos + 1);
    }

    CharString fullPath =  MetroFileSystem::Paths::MeshesFolder + modelPath + ".model";
    MetroFSPath file = MetroContext::Get().GetFilesystem().FindFile(fullPath);
    if (file.IsValid()) {
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(file);
        if (stream.Good()) {
            result = this->LoadFromData(stream, file, needAnimations);
            if (result && !tpreset.empty()) {
                this->SetTPreset(tpreset);
            }
        }
    }

    return result;
}

bool MetroModel::LoadFromData(MemStream& stream, const MetroFSPath& fileIdx, const bool needAnimations) {
    bool result = false;

#if 0
    RefPtr<MetroModelBase> test = MetroModelFactory::CreateModelFromStream(stream, MetroModelLoadParams::LoadGeometry);
#endif

    mThisFileIdx = fileIdx;

    this->ReadSubChunks(stream);

    this->ReplaceTextures();

    if (needAnimations) {
        this->LoadMotions();
    }

    result = !mMeshes.empty();

    return result;
}


bool MetroModel::IsAnimated() const {
    bool hasAnimMesh = false;
    for (auto m : mMeshes) {
        hasAnimMesh = hasAnimMesh || m->skinned;
    }

    if (!mSkeleton && hasAnimMesh) {
        return true;
    }

    return mSkeleton != nullptr;
}

const AABBox& MetroModel::GetBBox() const {
    return mBBox;
}

const BSphere& MetroModel::GetBSphere() const {
    return mBSphere;
}

size_t MetroModel::GetNumMeshes() const {
    return mMeshes.size();
}

const MetroMesh* MetroModel::GetMesh(const size_t idx) const {
    return mMeshes[idx];
}

MyArray<MetroVertex> MetroModel::MakeCommonVertices(const size_t meshIdx) const {
    MyArray<MetroVertex> result;

    const MetroMesh* mesh = this->GetMesh(meshIdx);
    if (this->IsAnimated()) {
        const VertexSkinned* srcVerts = rcast<const VertexSkinned*>(mesh->rawVB.data());

        result.resize(mesh->numVertices);
        MetroVertex* dstVerts = result.data();

        for (size_t i = 0; i < mesh->numVertices; ++i) {
            *dstVerts = ConvertVertex(*srcVerts);
            dstVerts->pos *= mesh->vscale;
            ++srcVerts;
            ++dstVerts;
        }
    } else {
        const VertexStatic* srcVerts = rcast<const VertexStatic*>(mesh->rawVB.data());

        result.resize(mesh->numVertices);
        MetroVertex* dstVerts = result.data();

        for (size_t i = 0; i < mesh->numVertices; ++i) {
            *dstVerts = ConvertVertex(*srcVerts);
            //dstVerts->pos *= mesh->vscale;
            ++srcVerts;
            ++dstVerts;
        }
    }

    return std::move(result);
}

const CharString& MetroModel::GetSkeletonPath() const {
    return mSkeletonPath;
}

const MetroSkeleton* MetroModel::GetSkeleton() const {
    return mSkeleton;
}

MetroModel* MetroModel::GetLodModel(const size_t idx) const {
    if (idx < kMetroModelMaxLods) {
        return mLodModels[idx];
    } else {
        return nullptr;
    }
}

size_t MetroModel::GetNumMotions() const {
    return mMotions.size();
}

CharString MetroModel::GetMotionName(const size_t idx) const {
    const MetroFSPath& file = mMotions[idx].file;
    const CharString& fileName = MetroContext::Get().GetFilesystem().GetName(file);

    CharString name = fileName.substr(0, fileName.length() - 3);
    return name;
}

const CharString& MetroModel::GetMotionPath(const size_t idx) const {
    return mMotions[idx].path;
}

float MetroModel::GetMotionDuration(const size_t idx) const {
    return scast<float>(mMotions[idx].numFrames) / scast<float>(MetroMotion::kFrameRate);
}

const MetroMotion* MetroModel::GetMotion(const size_t idx) {
    MetroMotion* motion = mMotions[idx].motion;

    if (!motion) {
        const CharString& name = this->GetMotionName(idx);
        const MetroFSPath& file = mMotions[idx].file;

        motion = new MetroMotion(name);
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(file);

        if (MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033) {
            motion->LoadFromData_2033(stream);
        } else {
            motion->LoadFromData(stream);
        }

        mMotions[idx].motion = motion;
    }

    return motion;
}

const CharString& MetroModel::GetTextureReplacement(const HashString& name) const {
    auto it = mTextureReplacements.find(name);
    if (it != mTextureReplacements.end()) {
        return it->second;
    } else {
        return name.str;
    }
}

const CharString& MetroModel::GetComment() const {
    return mComment;
}


static void FixAndRemapBones(VertexSkinned& v, const BytesArray& remap) {
    v.bones[0] = remap[v.bones[0] / 3];
    v.bones[1] = remap[v.bones[1] / 3];
    v.bones[2] = remap[v.bones[2] / 3];
    v.bones[3] = remap[v.bones[3] / 3];
}

void MetroModel::ReadSubChunks(MemStream& stream) {
    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case MC_HeaderChunk: {
                MdlHeader hdr;
                stream.ReadStruct(hdr);

                if (hdr.vscale <= MM_Epsilon) {
                    hdr.vscale = 1.0f;
                }

                if (mCurrentMesh) {
                    mCurrentMesh->version = hdr.version;
                    mCurrentMesh->flags = hdr.flags;
                    mCurrentMesh->vscale = hdr.vscale;
                    mCurrentMesh->bbox.minimum = MetroSwizzle(hdr.bbox.minimum);
                    mCurrentMesh->bbox.maximum = MetroSwizzle(hdr.bbox.maximum);
                    mCurrentMesh->type = hdr.type;
                    mCurrentMesh->shaderId = hdr.shaderId;
                } else {
                    mVersion = hdr.version;
                    mBBox.minimum = MetroSwizzle(hdr.bbox.minimum);
                    mBBox.maximum = MetroSwizzle(hdr.bbox.maximum);
                    mBSphere.center = MetroSwizzle(hdr.bsphere.center);
                    mBSphere.radius = hdr.bsphere.radius;
                    mType = hdr.type;
                }
            } break;

            case MC_MaterialsChunk: {
                if (!mCurrentMesh) {
                    return;
                }

                mCurrentMesh->materials.resize(kMetroModelMaxMaterials);
                mCurrentMesh->materials[0] = stream.ReadStringZ();  // texture name
                mCurrentMesh->materials[1] = stream.ReadStringZ();  // shader name
                mCurrentMesh->materials[2] = stream.ReadStringZ();  // game material name
                if (mCurrentMesh->version >= kModelVersionLastLight) {
                    mCurrentMesh->materials[3] = stream.ReadStringZ();
                }

                //#NOTE_SK: here's the game, depending on some other flags, might read uint16_t
                //          uint16_t matFlags = materialsStream.ReadU16();
                //          bool ignore_model = matFlags & 8;
                const uint16_t flags0 = stream.ReadU16();
                const uint16_t flags1 = stream.ReadU16();

                //#NOTE_SK: seems like meshes with either "invalid" texture, and/or "collision" source materials
                //          are additional collision geometry, invisible during drawing
                //const CharString& textureName = mCurrentMesh->materials[0];
                //const CharString& shaderName = mCurrentMesh->materials[1];
                //const CharString& srcMatName = mCurrentMesh->materials[3];
                //const bool noCollision = StrContains(srcMatName, "no collision");
                //if (!noCollision &&
                //    (StrEndsWith(textureName, "invalid") ||
                //     StrContains(shaderName, "invisible") ||
                //     StrContains(srcMatName, "collision") ||
                //     StrContains(srcMatName, "colision"))) {
                    //mCurrentMesh->isCollision = true;
                    //LogPrintF(LogLevel::Info, "collision detected, flags0 = 0x%04x, flags1 = 0x%04x", flags0, flags1);

                    if (flags0 & 8) {
                        //LogPrintF(LogLevel::Info, "collision detected, texture = %s, shader = %s, srcMat = %s", textureName.c_str(), shaderName.c_str(), srcMatName.c_str());
                        mCurrentMesh->isCollision = true;
                    }
                //}

            } break;

            case MC_VerticesChunk: {
                if (mCurrentMesh) {
                    mCurrentMesh->skinned = false;

                    const size_t vertexType = stream.ReadTyped<uint32_t>();
                    const size_t numVertices = stream.ReadTyped<uint32_t>();
                    const size_t numShadowVertices = mCurrentMesh->version >= kModelVersionArktika1 ? stream.ReadTyped<uint16_t>() : 0;

                    mCurrentMesh->numVertices = numVertices;

                    const size_t vbSize = numVertices * sizeof(VertexStatic);
                    mCurrentMesh->rawVB.resize(vbSize);

                    stream.ReadToBuffer(mCurrentMesh->rawVB.data(), vbSize);
                }
            } break;

            case MC_SkinnedVerticesChunk: {
                if (mCurrentMesh) {
                    mCurrentMesh->skinned = true;

                    const size_t numBones = stream.ReadTyped<uint8_t>();

                    mCurrentMesh->bonesRemap.resize(numBones);
                    stream.ReadToBuffer(mCurrentMesh->bonesRemap.data(), numBones);

                    mCurrentMesh->hitBoxes.resize(numBones);
                    stream.ReadToBuffer(mCurrentMesh->hitBoxes.data(), numBones * sizeof(MetroOBB));

                    size_t numVertices = 0, numShadowVertices = 0;

                    numVertices = stream.ReadTyped<uint32_t>();
                    if (mCurrentMesh->version >= kModelVersionArktika1) {
                        numShadowVertices = stream.ReadTyped<uint16_t>();
                    } else {
                        mCurrentMesh->vscale = 12.0f;   //#NOTE_SK: Redux and Original versions are in range [-12 .. 12]
                    }

                    mCurrentMesh->numVertices = numVertices;

                    const size_t vbSize = numVertices * sizeof(VertexSkinned);
                    mCurrentMesh->rawVB.resize(vbSize);

                    stream.ReadToBuffer(mCurrentMesh->rawVB.data(), vbSize);

                    VertexSkinned* vertices = rcast<VertexSkinned*>(mCurrentMesh->rawVB.data());
                    for (size_t i = 0; i < numVertices; ++i) {
                        FixAndRemapBones(vertices[i], mCurrentMesh->bonesRemap);
                    }
                }
            } break;

            case MC_FacesChunk: {
                if (mCurrentMesh) {
                    size_t numFaces = 0, numShadowFaces = 0;

                    if (!mCurrentMesh->skinned) {
                        numFaces = stream.ReadTyped<uint32_t>();
                        if (mCurrentMesh->version >= kModelVersionArktika1) {
                            numShadowFaces = stream.ReadTyped<uint16_t>();
                        }

                        if (mCurrentMesh->version < kModelVersionArktika1) {
                            numFaces /= 3;  //#NOTE_SK: Redux and Original models store number of indices, not faces
                            numShadowFaces /= 3;
                        }
                    } else {
                        numFaces = stream.ReadTyped<uint16_t>();
                        numShadowFaces = stream.ReadTyped<uint16_t>();

                        if (mCurrentMesh->version < kModelVersionLastLight) {
                            numFaces /= 3;  //#NOTE_SK: 2033 models store number of indices, not faces
                            numShadowFaces /= 3;
                        }
                    }

                    mCurrentMesh->faces.resize(numFaces);
                    stream.ReadToBuffer(mCurrentMesh->faces.data(), numFaces * sizeof(MetroFace));
                }
            } break;

            case MC_ChildrenChunk: {
                MemStream meshesStream = stream.Substream(chunkSize);
                size_t nextMeshId = 0;
                while (!meshesStream.Ended()) {
                    const size_t subMeshId = meshesStream.ReadTyped<uint32_t>();
                    const size_t subMeshSize = meshesStream.ReadTyped<uint32_t>();

                    if (subMeshId == nextMeshId) {
                        mCurrentMesh = new MetroMesh();
                        mMeshes.push_back(mCurrentMesh);

                        MemStream subStream = meshesStream.Substream(subMeshSize);
                        this->ReadSubChunks(subStream);
                        ++nextMeshId;

                        meshesStream.SkipBytes(subMeshSize);
                    } else {
                        break;
                    }
                }
                mCurrentMesh = nullptr;
            } break;

            case MC_Lod_1_Chunk: {
                if (mLodModels[0] != nullptr) {
                    MySafeDelete(mLodModels[0]);
                }
                MetroModel* model = new MetroModel();
                if (model->LoadFromData(stream, mThisFileIdx)) {
                    mLodModels[0] = model;
                } else {
                    MySafeDelete(model);
                }
            } break;

            case MC_Lod_2_Chunk: {
                if (mLodModels[1] != nullptr) {
                    MySafeDelete(mLodModels[1]);
                }
                MetroModel* model = new MetroModel();
                if (model->LoadFromData(stream, mThisFileIdx)) {
                    mLodModels[1] = model;
                } else {
                    MySafeDelete(model);
                }
            } break;

            case MC_MeshesInline: {
                stream.SkipBytes(16); // wtf ???
                MemStream subStream = stream.Substream(chunkSize - 16);
                this->ReadSubChunks(subStream);
            } break;

            case MC_MeshesLinks: {
                const size_t numStrings = stream.ReadTyped<uint32_t>();
                StringArray links, linksLod1, linksLod2;
                for (size_t i = 0; i < numStrings; ++i) {
                    CharString linksString = stream.ReadStringZ();
                    if (!linksString.empty()) {
                        StringArray splittedLinks = StrSplit(linksString, ',');
                        if (this->IsAnimated() && numStrings <= 3) { // is it correct to find lods of dynamic models this way?
                            if (i == 1) {
                                linksLod1.insert(linksLod1.end(), splittedLinks.begin(), splittedLinks.end());
                            } else if (i == 2) {
                                linksLod2.insert(linksLod2.end(), splittedLinks.begin(), splittedLinks.end());
                            } else {
                                links.insert(links.end(), splittedLinks.begin(), splittedLinks.end());
                            }
                        } else {
                            links.insert(links.end(), splittedLinks.begin(), splittedLinks.end());
                        }
                    }
                }

                if (!links.empty()) {
                    this->LoadLinkedMeshes(links);
                }
                if (!linksLod1.empty()) {
                    if (mLodModels[0] == nullptr) {
                        mLodModels[0] = new MetroModel();
                        mLodModels[0]->mThisFileIdx = this->mThisFileIdx;
                    }
                    mLodModels[0]->LoadLinkedMeshes(linksLod1);
                }
                if (!linksLod2.empty()) {
                    if (mLodModels[1] == nullptr) {
                        mLodModels[1] = new MetroModel();
                        mLodModels[1]->mThisFileIdx = this->mThisFileIdx;
                    }
                    mLodModels[1]->LoadLinkedMeshes(linksLod2);
                }
            } break;

            case MC_SkeletonLink: {
                CharString skeletonRef = stream.ReadStringZ();
                mSkeletonPath = MetroFileSystem::Paths::MeshesFolder + skeletonRef + MetroContext::Get().GetSkeletonExtension();
                const MetroFSPath file = MetroContext::Get().GetFilesystem().FindFile(mSkeletonPath);
                if (file.IsValid()) {
                    MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(file);
                    if (stream) {
                        mSkeleton = new MetroSkeleton();
                        const bool result = (MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033) ?
                                                mSkeleton->LoadFromData_2033(stream) : mSkeleton->LoadFromData(stream);
                        if (!result) {
                            MySafeDelete(mSkeleton);
                        }
                    }
                }
            } break;

            case MC_SkeletonInline: {
                mSkeleton = new MetroSkeleton();
                MemStream skelStream = stream.Substream(chunkSize);
                const bool result = (MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033) ?
                                        mSkeleton->LoadFromData_2033(skelStream) : mSkeleton->LoadFromData(skelStream);
                if (!result) {
                    MySafeDelete(mSkeleton);
                }
            } break;

            case MC_TexturesReplacements: {
                const CharString& replacementsString = stream.ReadStringZ();
                const StringArray& splits = StrSplit(replacementsString, ',');
                if (!splits.empty()) {
                    mTextureReplacements.reserve(splits.size());
                    for (const auto& s : splits) {
                        const size_t equalPos = s.find_first_of('=');
                        if (CharString::npos != equalPos) {
                            mTextureReplacements[s.substr(0, equalPos)] = s.substr(equalPos + 1);
                        }
                    }
                }
            } break;

            case MC_TexturesPresets: {
                const size_t numPresets = stream.ReadTyped<uint16_t>();
                mTexturePresets.resize(numPresets);
                for (auto& preset : mTexturePresets) {
                    preset.name = stream.ReadStringZ();
                    preset.hit_preset = stream.ReadStringZ();
                    if (mVersion >= kModelVersionLastLightRelease) {
                        preset.voice = stream.ReadStringZ();
                    }
                    if (mVersion >= kModelVersionRedux) {
                        preset.flags = stream.ReadTyped<uint32_t>();
                    }

                    const size_t numItems = stream.ReadTyped<uint16_t>();
                    preset.items.resize(numItems);
                    for (auto& item : preset.items) {
                        item.mtl_name = stream.ReadStringZ();
                        item.t_dst = stream.ReadStringZ();
                        item.s_dst = stream.ReadStringZ();
                    }
                }
            } break;

            case MC_Comment: {
                if (chunkSize > 15) {
                    stream.SkipBytes(15); // wtf ???
                    CharString c0 = stream.ReadStringZ();
                    CharString c1 = stream.ReadStringZ();
                    mComment = c0.empty() ? c1 : (c0 + "\n\n" + c1);
                }
            } break;
        }

        stream.SetCursor(chunkEnd);
    }
}

void MetroModel::LoadLinkedMeshes(const StringArray& links) {
    mCurrentMesh = nullptr;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
    for (const CharString& lnk : links) {
        MetroFSPath file(MetroFSPath::Invalid);

        if (lnk[0] == '.' && lnk[1] == kPathSeparator) { // relative path
            const MetroFSPath folder = mfs.GetParentFolder(mThisFileIdx);
            file = mfs.FindFile(lnk.substr(2) + ".mesh", folder);
        } else {
            CharString meshFilePath = MetroFileSystem::Paths::MeshesFolder + lnk + ".mesh";
            file = mfs.FindFile(meshFilePath);
        }
        if (file.IsValid()) {
            MemStream stream = mfs.OpenFileStream(file);
            if (stream) {
                this->ReadSubChunks(stream);
            }

            mCurrentMesh = nullptr;
        }
    }
}

void MetroModel::ReplaceTextures() {
    for (MetroMesh* m : mMeshes) {
        CharString& ts = m->materials.front();
        ts = this->GetTextureReplacement(ts);
    }
}

void MetroModel::SetTPreset(const CharString& tpreset) {
    const auto it = std::find_if(mTexturePresets.begin(), mTexturePresets.end(), [&tpreset](const TexturePreset& p)->bool {
        return p.name == tpreset;
    });

    if (it != mTexturePresets.end()) {
        const TexturePreset& preset = *it;
        for (MetroMesh* m : mMeshes) {
            const CharString& mname = m->materials[3];
            if (!mname.empty()) {
                const auto iit = std::find_if(preset.items.begin(), preset.items.end(), [&mname](const TexturePreset::Item& item)->bool {
                    return item.mtl_name == mname;
                });

                if (iit != preset.items.end()) {
                    const TexturePreset::Item& item = *iit;
                    if (!item.t_dst.empty()) {
                        m->materials[0] = item.t_dst;
                    }
                    if (!item.s_dst.empty()) {
                        m->materials[1] = item.s_dst;
                    }
                }
            }
        }
    }
}

void MetroModel::LoadMotions() {
    CharString motionsStr;
    if (mSkeleton) {
        motionsStr = mSkeleton->GetMotionsStr();
    }

    if (motionsStr.empty()) {
        return;
    }

    const bool is2033 = MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    MyArray<MetroFSPath> motionFiles;

    StringArray motionFolders = StrSplit(motionsStr, ',');
    StringArray motionPaths;
    const CharString& motionsExt = MetroContext::Get().GetMotionExtension();
    for (const CharString& f : motionFolders) {
        CharString fullFolderPath = MetroFileSystem::Paths::MotionsFolder + f + "\\";

        const auto& files = mfs.FindFilesInFolder(fullFolderPath, motionsExt);

        for (const auto& file : files) {
            motionPaths.push_back(fullFolderPath + mfs.GetName(file));
        }

        motionFiles.insert(motionFiles.end(), files.begin(), files.end());
    }

    const size_t numBones = mSkeleton->GetNumBones();

    mMotions.reserve(motionFiles.size());
    size_t i = 0;
    for (const MetroFSPath& fp : motionFiles) {
        MemStream stream = mfs.OpenFileStream(fp);
        if (stream) {
            MetroMotion motion(kEmptyString);
            const bool loaded = is2033 ? motion.LoadHeader_2033(stream) : motion.LoadHeader(stream);
            if (loaded && motion.GetNumBones() == numBones) {
                mMotions.push_back({fp, motion.GetNumFrames(), motionPaths[i], nullptr});
            }
        }
        ++i;
    }
}
