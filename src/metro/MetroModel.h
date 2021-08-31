#pragma once
#include "MetroTypes.h"

class MetroSkeleton;
class MetroMotion;

struct MetroModelLoadParams {
    enum LoadFlags : uint32_t {
        LoadGeometry    = 1,
        LoadCollision   = 2,
        LoadSkeleton    = 4,
        LoadTPresets    = 8,

        LoadForceSkin   = 0x10000,

        LoadEverything  = 0xF
    };

    CharString  modelName;
    CharString  tpresetName;
    uint32_t    formatVersion;
    uint32_t    loadFlags;
    MetroFSPath srcFile;
};


struct MetroModelTPreset {
    struct Item {
        CharString  mtl_name;
        CharString  t_dst;
        CharString  s_dst;
    };

    CharString      name;
    CharString      hit_preset;
    CharString      voice;
    uint32_t        flags;  // lowest bit is use_as_modifier
    MyArray<Item>   items;
};


struct MetroModelMesh {
    MetroModelMesh()
        : verticesOffset(0u)
        , verticesCount(0u)
        , indicesOffset(0u)
        , facesCount(0u)
        , shadowVerticesCount(0u)
        , shadowFacesCount(0u)
        , vertexType(MetroVertexType::Invalid)
        , verticesScale(1.0f)
    { }

    uint32_t    verticesOffset;
    uint32_t    verticesCount;
    uint32_t    indicesOffset;
    uint32_t    facesCount;
    uint32_t    shadowVerticesCount;
    uint32_t    shadowFacesCount;
    uint32_t    vertexType;
    float       verticesScale;
    BytesArray  bonesRemap;
};

class MetroModelBase;
class MetroModelStd;
class MetroModelSkin;
class MetroModelHierarchy;
class MetroModelSkeleton;
class MetroModelSoft;
class MetroClothModel;

struct MetroModelGeomData {
    AABBox                  bbox;
    BSphere                 bsphere;
    StringView              texture;
    const MetroModelMesh*   mesh;
    const MetroModelBase*   model;
    const void*             vertices;
    const void*             faces;
};

enum class MetroModelType : size_t {
    Std             = 0,
    Hierarchy       = 1,
    Skeleton        = 2,
    Skeleton2       = 3,    // wtf ???
    Hierarchy2      = 4,    // wtf ???
    Skin            = 5,
    Soft            = 8,
    ParticlesEffect = 11,
    ParticlesSystem = 12,
    Skeleton3       = 13    // wtf ???
};



// Base class for all Metro models
class MetroModelBase {
    friend class MetroModelStd;
    friend class MetroModelSkin;
    friend class MetroModelHierarchy;
    friend class MetroModelSkeleton;
    friend class MetroModelSoft;

public:
    MetroModelBase();
    virtual ~MetroModelBase();

    virtual bool                    Load(MemStream& stream, MetroModelLoadParams& params);
    virtual bool                    Save(MemWriteStream& stream);

    void                            SetSourceName(const CharString& srcName);
    const CharString&               GetSourceName() const;

    MetroModelType                  GetModelType() const;
    size_t                          GetModelVersion() const;
    void                            SetModelVersion(const size_t version);

    virtual uint32_t                GetCheckSum() const;
    virtual size_t                  GetLodCount() const;

    const AABBox&                   GetBBox() const;
    void                            SetBBox(const AABBox& bbox);
    const BSphere&                  GetBSphere() const;
    void                            SetBSphere(const BSphere& bsphere);

    const CharString&               GetMaterialString(const size_t idx) const;
    void                            SetMaterialString(const CharString& str, const size_t idx);

    bool                            IsCollisionModel() const;

    virtual bool                    MeshValid() const;
    virtual RefPtr<MetroModelMesh>  GetMesh() const;

    virtual size_t                  GetVerticesCount() const;
    virtual size_t                  GetFacesCount() const;
    virtual uint32_t                GetVertexType() const;

    virtual size_t                  GetVerticesMemSize() const  { return 0; }
    virtual const void*             GetVerticesMemData() const  { return nullptr; }
    virtual size_t                  GetFacesMemSize() const     { return 0; }
    virtual const void*             GetFacesMemData() const     { return nullptr; }

    virtual void                    FreeGeometryMem()           { }

    virtual void                    CollectGeomData(MyArray<MetroModelGeomData>& result, const size_t lodIdx = kInvalidValue) const;

    virtual bool                    IsSkeleton() const { return false; }
    virtual bool                    IsHierarchy() const { return false; }
    virtual bool                    IsSoft() const { return false; }

protected:
    virtual void                    ApplyTPresetInternal(const MetroModelTPreset& tpreset);

protected:
    uint16_t                        mVersion;
    uint16_t                        mType;
    uint32_t                        mFlags;
    uint32_t                        mEngineMtl;
    uint32_t                        mChecksum;
    float                           mSSABias;
    AABBox                          mBBox;
    BSphere                         mBSphere;
    RefPtr<MetroModelMesh>          mMesh;
    StringArray                     mMaterialStrings;
    uint16_t                        mMaterialFlags0;
    uint16_t                        mMaterialFlags1;
    bool                            mIsCollisionModel;
    CharString                      mSourceName;
};

// Simple static model, could be just a *.mesh file
class MetroModelStd final : public MetroModelBase {
    INHERITED_CLASS(MetroModelBase);
public:
    MetroModelStd();
    virtual ~MetroModelStd();

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream) override;

    virtual size_t          GetVerticesMemSize() const override;
    virtual const void*     GetVerticesMemData() const override;
    virtual size_t          GetFacesMemSize() const override;
    virtual const void*     GetFacesMemData() const override;

    virtual void            FreeGeometryMem() override;

    // model creation
    void                    CreateMesh(const size_t numVertices, const size_t numFaces);
    void                    CopyVerticesData(const void* vertices);
    void                    CopyFacesData(const void* faces);
    void                    SetBounds(const AABBox& bbox, const BSphere& bsphere);

protected:
    BytesArray              mVerticesData;
    BytesArray              mFacesData;
};

// Simple skinned model, could be just a *.mesh file
class MetroModelSkin final : public MetroModelBase {
    INHERITED_CLASS(MetroModelBase);
public:
    MetroModelSkin();
    virtual ~MetroModelSkin();

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream) override;

    virtual size_t          GetVerticesMemSize() const override;
    virtual const void*     GetVerticesMemData() const override;
    virtual size_t          GetFacesMemSize() const override;
    virtual const void*     GetFacesMemData() const override;

    virtual void            FreeGeometryMem() override;

    MetroModelSkeleton*     GetParent() const;
    void                    SetParent(MetroModelSkeleton* parent);

protected:
    MetroModelSkeleton*     mParent;
    MyArray<MetroOBB>       mBonesOBB;
    BytesArray              mVerticesData;
    BytesArray              mFacesData;
};

// Complete static model, can consist of multiple Std models (inline or external *.mesh files)
class MetroModelHierarchy : public MetroModelBase {
    INHERITED_CLASS(MetroModelBase);
public:
    MetroModelHierarchy();
    virtual ~MetroModelHierarchy();

    virtual bool                Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool                Save(MemWriteStream& stream) override;

    virtual size_t              GetLodCount() const override;

    virtual void                FreeGeometryMem() override;

    virtual void                CollectGeomData(MyArray<MetroModelGeomData>& result, const size_t lodIdx = kInvalidValue) const override;

    virtual bool                IsHierarchy() const { return true; }

    void                        ApplyTPreset(const CharString& tpresetName);

    size_t                      GetChildrenCount() const;
    RefPtr<MetroModelBase>      GetChild(const size_t idx) const;

    size_t                      GetChildrenRefsCount() const;
    uint32_t                    GetChildRef(const size_t idx) const;

    void                        AddChild(const RefPtr<MetroModelBase>& child);

protected:
    void                        LoadTPresets(const StreamChunker& chunker);
    void                        SaveTPresets(MemWriteStream& stream);
    virtual void                ApplyTPresetInternal(const MetroModelTPreset& tpreset) override;

protected:
    using ModelPtr = RefPtr<MetroModelBase>;

    MyArray<ModelPtr>           mChildren;
    MyArray<uint32_t>           mChildrenRefs;
    MyArray<ModelPtr>           mLods;
    MyArray<MetroModelTPreset>  mTPresets;
};

// Complete animated model, can consist of multiple Skin models (inline or external *.mesh files)
class MetroModelSkeleton final : public MetroModelHierarchy {
    INHERITED_CLASS(MetroModelHierarchy);
public:
    MetroModelSkeleton();
    virtual ~MetroModelSkeleton();

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream) override;

    virtual size_t          GetLodCount() const override;

    virtual bool            IsSkeleton() const override { return true; }

    const RefPtr<MetroSkeleton>& GetSkeleton() const;
    void                    SetSkeleton(RefPtr<MetroSkeleton> skeleton);

protected:
    bool                    LoadLodMeshes(MetroModelHierarchy* target, CharString& meshesNames, MetroModelLoadParams& params, const size_t lodIdx);
    bool                    LoadLodMeshes(MetroModelHierarchy* target, MemStream& meshesStream, MetroModelLoadParams& params, const size_t lodIdx);
    bool                    LoadLodMesh(MetroModelHierarchy* target, MemStream& stream, MetroModelLoadParams& params, const size_t lodIdx);

protected:
    using ModelPtr = RefPtr<MetroModelBase>;
    using LodMeshesArr = MyArray<ModelPtr>;

    RefPtr<MetroSkeleton>   mSkeleton;
    MyArray<LodMeshesArr>   mLodMeshes;
};

class MetroModelSoft final : public MetroModelBase {
    INHERITED_CLASS(MetroModelBase);
public:
    MetroModelSoft();
    virtual ~MetroModelSoft();

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream) override;

    virtual size_t          GetVerticesMemSize() const override;
    virtual const void*     GetVerticesMemData() const override;
    virtual size_t          GetFacesMemSize() const override;
    virtual const void*     GetFacesMemData() const override;

    virtual void            FreeGeometryMem() override;

    virtual bool            IsSoft() const override { return true; }

protected:
    RefPtr<MetroClothModel> mClothModel;
};


class MetroClothModel {
public:
    MetroClothModel();
    ~MetroClothModel();

    bool                Load(MemStream& stream);

    size_t              GetVerticesCount() const;
    const VertexSoft*   GetVertices() const;

    size_t              GetIndicesCount() const;
    const uint16_t*     GetIndices() const;

private:
    uint32_t            mFormat;
    uint32_t            mChecksum;
    // params
    float               mTearingFactor;
    float               mBendStiffness;
    float               mStretchStiffness;
    float               mDensity;
    bool                mTearable;
    bool                mApplyPressure;
    bool                mApplyWelding;
    float               mPressure;
    float               mWeldingDistance;
    // geometry
    MyArray<VertexSoft> mVertices;
    MyArray<uint16_t>   mIndices;
};


class MetroModelFactory {
    MetroModelFactory() = delete;
    MetroModelFactory(const MetroModelFactory&) = delete;
    MetroModelFactory(MetroModelFactory&&) = delete;

public:
    static RefPtr<MetroModelBase>   CreateModelFromType(const MetroModelType type);
    static RefPtr<MetroModelBase>   CreateModelFromStream(MemStream& stream, const MetroModelLoadParams& params);
    static RefPtr<MetroModelBase>   CreateModelFromFile(const MetroFSPath& file, const uint32_t loadFlags);
    static RefPtr<MetroModelBase>   CreateModelFromFullName(const CharString& fullName, const uint32_t loadFlags);
};

