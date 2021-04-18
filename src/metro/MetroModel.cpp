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

PACKED_STRUCT_BEGIN
struct VertexSoftLegacy {
    vec3     pos;
    uint32_t aux0;
    uint32_t aux1;
    vec3     normal;
    int16_t  uv[2];
} PACKED_STRUCT_END;
static_assert(sizeof(VertexSoftLegacy) == 36);


// Base class for all Metro models
MetroModelBase::MetroModelBase()
    : mVersion(0)
    , mType(0)
    , mFlags(0)
    , mEngineMtl(0xFFFF)
    , mChecksum(0)
    , mSSABias(0.0f)
    , mBBox{}
    , mBSphere{}
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

void MetroModelBase::ApplyTPresetInternal(const MetroModelTPreset& tpreset) {
    if (this->MeshValid()) {
        const CharString& mname = mMaterialStrings[3];
        if (!mname.empty()) {
            const auto iit = std::find_if(tpreset.items.begin(), tpreset.items.end(), [&mname](const MetroModelTPreset::Item& item)->bool {
                return item.mtl_name == mname;
            });

            if (iit != tpreset.items.end()) {
                const MetroModelTPreset::Item& item = *iit;
                if (!item.t_dst.empty()) {
                    mMaterialStrings[0] = item.t_dst;
                }
                if (!item.s_dst.empty()) {
                    mMaterialStrings[1] = item.s_dst;
                }
            }
        }
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
        if (params.formatVersion >= kModelVersionEarlyArktika1) {
            mMesh->shadowVerticesCount = meshRefStream.ReadU32();
        }

        mMesh->indicesOffset = meshRefStream.ReadU32();
        mMesh->facesCount = meshRefStream.ReadU32() / 3;
        if (params.formatVersion >= kModelVersionEarlyArktika1) {
            mMesh->shadowFacesCount = meshRefStream.ReadU32() / 3;
        }

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
        if (TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadTPresets) || !params.tpresetName.empty()) {
            this->LoadTPresets(chunker);
        }

        MemStream childrenRefsStream = chunker.GetChunkStream(MC_ChildrenRefsChunk);
        if (childrenRefsStream) {
            const size_t meshesCount = childrenRefsStream.ReadU32();
            mChildrenRefs.resize(meshesCount);
            childrenRefsStream.ReadToBuffer(mChildrenRefs.data(), meshesCount * sizeof(uint32_t));

            result = true;
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
                        RefPtr<MetroModelBase> child = MetroModelFactory::CreateModelFromStream(childStream, params);
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
            RefPtr<MetroModelBase> lod1Model = lod1Stream ? MetroModelFactory::CreateModelFromStream(lod1Stream, params) : nullptr;

            MemStream lod2Stream = chunker.GetChunkStream(MC_Lod_2_Chunk);
            RefPtr<MetroModelBase> lod2Model = lod2Stream ? MetroModelFactory::CreateModelFromStream(lod2Stream, params) : nullptr;

            if (lod1Model) {
                mLods.push_back(lod1Model);
            }
            if (lod2Model) {
                mLods.push_back(lod2Model);
            }
        }

        if (!params.tpresetName.empty()) {
            this->ApplyTPreset(params.tpresetName);
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

void MetroModelHierarchy::ApplyTPreset(const CharString& tpresetName) {
    const auto it = std::find_if(mTPresets.begin(), mTPresets.end(), [&tpresetName](const MetroModelTPreset& p)->bool {
        return p.name == tpresetName;
    });

    if (it != mTPresets.end()) {
        this->ApplyTPresetInternal(*it);
    }
}

size_t MetroModelHierarchy::GetChildrenCount() const {
    return mChildren.size();
}

RefPtr<MetroModelBase> MetroModelHierarchy::GetChild(const size_t idx) const {
    return mChildren[idx];
}

size_t MetroModelHierarchy::GetChildrenRefsCount() const {
    return mChildrenRefs.size();
}

uint32_t MetroModelHierarchy::GetChildRef(const size_t idx) const {
    return mChildrenRefs[idx];
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

void MetroModelHierarchy::LoadTPresets(const StreamChunker& chunker) {
    MemStream tpresetsStream = chunker.GetChunkStream(MC_TexturesPresets);
    if (tpresetsStream) {
        const size_t numPresets = tpresetsStream.ReadU16();
        mTPresets.resize(numPresets);
        for (auto& preset : mTPresets) {
            preset.name = tpresetsStream.ReadStringZ();
            if (mVersion >= 12) {
                preset.hit_preset = tpresetsStream.ReadStringZ();
            }
            if (mVersion >= 21) {
                preset.voice = tpresetsStream.ReadStringZ();
            }
            if (mVersion >= 23) {
                preset.flags = tpresetsStream.ReadU32();
            }

            const size_t numItems = tpresetsStream.ReadU16();
            preset.items.resize(numItems);
            for (auto& item : preset.items) {
                item.mtl_name = tpresetsStream.ReadStringZ();
                item.t_dst = tpresetsStream.ReadStringZ();
                item.s_dst = tpresetsStream.ReadStringZ();
            }
        }
    }
}

void MetroModelHierarchy::ApplyTPresetInternal(const MetroModelTPreset& tpreset) {
    for (auto& child : mChildren) {
        child->ApplyTPresetInternal(tpreset);
    }

    for (auto& lod : mChildren) {
        lod->ApplyTPresetInternal(tpreset);
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

    if (TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadTPresets) || !params.tpresetName.empty()) {
        this->LoadTPresets(chunker);
    }

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

    if (!params.tpresetName.empty()) {
        this->ApplyTPreset(params.tpresetName);
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
                MetroModelLoadParams loadParams = params;
                loadParams.srcFile = file;
                if (!this->LoadLodMesh(target, stream, loadParams, lodIdx)) {
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
        MetroModelLoadParams loadParams = params;
        //#NOTE_SK: for some weird reason some of the child meshes have empty header (all nulls, still 64 bytes)
        //          so we detect it as static mesh and fail miserably, so now we enforce skin mesh type
        loadParams.loadFlags |= MetroModelLoadParams::LoadForceSkin;
        RefPtr<MetroModelBase> mesh = MetroModelFactory::CreateModelFromStream(stream, loadParams);
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


MetroModelSoft::MetroModelSoft() {

}
MetroModelSoft::~MetroModelSoft() {

}

bool MetroModelSoft::Load(MemStream& stream, MetroModelLoadParams& params) {
    bool result = false;

    if (Base::Load(stream, params)) {
        MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
        MetroFSPath folder = mfs.GetParentFolder(params.srcFile);

        CharString fileName = mfs.GetName(params.srcFile);
        const CharString& clothExt = MetroContext::Get().GetClothModelExtension();

        CharString::size_type dotPos = fileName.find('.');
        if (dotPos != CharString::npos) {
            fileName = fileName.substr(0, dotPos);
        }

        MetroFSPath file = mfs.FindFile(fileName + clothExt, folder);
        if (file.IsValid()) {
            MemStream stream = mfs.OpenFileStream(file);

            mClothModel = MakeRefPtr<MetroClothModel>();
            if (mClothModel->Load(stream)) {
                mMesh = MakeRefPtr<MetroModelMesh>();

                mMesh->verticesCount = mClothModel->GetVerticesCount();
                mMesh->facesCount = mClothModel->GetIndicesCount() / 3;

                mMesh->vertexType = MetroVertexType::Soft;
                mMesh->verticesScale = 1.0f;

                result = true;
            }
        }
    }

    return result;
}

bool MetroModelSoft::Save(MemWriteStream& stream) {
    return false;
}

size_t MetroModelSoft::GetVerticesMemSize() const {
    return mClothModel->GetVerticesCount() * sizeof(VertexSoft);
}

const void* MetroModelSoft::GetVerticesMemData() const {
    return mClothModel->GetVertices();
}

size_t MetroModelSoft::GetFacesMemSize() const {
    return mClothModel->GetIndicesCount() * sizeof(uint16_t);
}

const void* MetroModelSoft::GetFacesMemData() const {
    return mClothModel->GetIndices();
}

void MetroModelSoft::FreeGeometryMem() {
}


MetroClothModel::MetroClothModel()
    : mFormat(0)
    , mChecksum(0)
    , mTearingFactor(0.0f)
    , mBendStiffness(0.0f)
    , mStretchStiffness(0.0f)
    , mDensity(0.0f)
    , mTearable(false)
    , mApplyPressure(false)
    , mApplyWelding(false)
    , mPressure(0.0f)
    , mWeldingDistance(0.0f) {
}
MetroClothModel::~MetroClothModel() {
}

bool MetroClothModel::Load(MemStream& stream) {
    const uint32_t magic = stream.ReadU32();
    if (0x001BDF01 != magic) {
        return false;
    }

    stream.SkipBytes(1);

    mFormat = stream.ReadU32();
    mChecksum = stream.ReadU32();

    if (mFormat > 0) {
        mTearable = stream.ReadBool();
        mTearingFactor = stream.ReadF32();
        mBendStiffness = stream.ReadF32();
        mStretchStiffness = stream.ReadF32();

        if (mFormat >= 2) {
            mDensity = stream.ReadF32();

            if (mFormat >= 3) {
                mApplyPressure = stream.ReadBool();
                mPressure = stream.ReadF32();
                mApplyWelding = stream.ReadBool();
                mWeldingDistance = stream.ReadF32();
            }
        }
    }

    const size_t numIndices = stream.ReadU32();
    const size_t numVertices = stream.ReadU32();
    const size_t vertexSize = stream.ReadU32();

    assert(vertexSize == sizeof(VertexSoft) || vertexSize == sizeof(VertexSoftLegacy));

    mIndices.resize(numIndices);
    stream.ReadToBuffer(mIndices.data(), numIndices * sizeof(uint16_t));

    mVertices.resize(numVertices);
    if (mFormat > 4) {
        assert(vertexSize == sizeof(VertexSoft));
        stream.ReadToBuffer(mVertices.data(), numVertices * sizeof(VertexSoft));
    } else {
        assert(vertexSize == sizeof(VertexSoftLegacy));
        // slowly reading legacy vertices and converting them
        VertexSoftLegacy legacy;
        for (VertexSoft& v : mVertices) {
            stream.ReadStruct(legacy);

            v.pos = legacy.pos;
            v.aux0 = legacy.aux0;
            v.normal = legacy.normal;
            v.uv[0] = legacy.uv[0];
            v.uv[1] = legacy.uv[1];
        }
    }

    return true;
}

size_t MetroClothModel::GetVerticesCount() const {
    return mVertices.size();
}

const VertexSoft* MetroClothModel::GetVertices() const {
    return mVertices.data();
}

size_t MetroClothModel::GetIndicesCount() const {
    return mIndices.size();
}

const uint16_t* MetroClothModel::GetIndices() const {
    return mIndices.data();
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
            result = MakeRefPtr<MetroModelSoft>();
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

RefPtr<MetroModelBase> MetroModelFactory::CreateModelFromStream(MemStream& stream, const MetroModelLoadParams& params) {
    RefPtr<MetroModelBase> result;

    StreamChunker chunker(stream);

    MemStream headerStream = chunker.GetChunkStream(MC_HeaderChunk);
    if (headerStream) {
        MdlHeader hdr;
        headerStream.ReadStruct(hdr);

        if (!hdr.type && TestBit<uint32_t>(params.loadFlags, MetroModelLoadParams::LoadForceSkin)) {
            hdr.type = scast<uint8_t>(MetroModelType::Skin);
        }

        result = MetroModelFactory::CreateModelFromType(scast<MetroModelType>(hdr.type));
        if (result) {
            MetroModelLoadParams loadParams = params;
            loadParams.formatVersion = hdr.version;

            const bool success = result->Load(stream, loadParams);
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
        MetroModelLoadParams params = {
            kEmptyString,
            kEmptyString,
            0,
            loadFlags,
            file
        };
        return MetroModelFactory::CreateModelFromStream(stream, params);
    } else {
        return nullptr;
    }
}

RefPtr<MetroModelBase> MetroModelFactory::CreateModelFromFullName(const CharString& fullName, const uint32_t loadFlags) {
    // break full name into:
    //  model name
    //  tpreset name
    //  modifier name (???)

    CharString modelName, tpresetName, modifierName;

    size_t atPos = fullName.find('@');
    if (atPos != CharString::npos) {
        modelName = fullName.substr(0, atPos);
        tpresetName = fullName.substr(atPos + 1);

        atPos = tpresetName.find('@');
        if (atPos != CharString::npos) {
            modifierName = tpresetName.substr(atPos + 1);
            tpresetName = tpresetName.substr(0, atPos);
        }
    } else {
        modelName = fullName;
    }

    CharString fullPath = MetroFileSystem::Paths::MeshesFolder + modelName + ".model";
    MetroFSPath file = MetroContext::Get().GetFilesystem().FindFile(fullPath);
    if (file.IsValid()) {
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(file);
        if (stream) {
            MetroModelLoadParams params = {
                modelName,
                tpresetName,
                0,
                loadFlags,
                file
            };

            return MetroModelFactory::CreateModelFromStream(stream, params);
        }
    }

    return nullptr;
}

