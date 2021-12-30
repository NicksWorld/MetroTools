#pragma once
#include "MetroTypes.h"

class MetroSkeleton;
class MetroMotion;

using MetroModelTReplacements = MyDict<CharString, CharString>;

struct MetroModelLoadParams {
    enum LoadFlags : uint32_t {
        LoadGeometry    = 1,
        LoadCollision   = 2,
        LoadSkeleton    = 4,
        LoadTPresets    = 8,

        LoadForceSkin   = 0x10000,
        LoadForceSkinH  = 0x20000,

        ClearLoadForceMask = ~(LoadForceSkin | LoadForceSkinH),

        LoadEverything  = 0xF
    };

    CharString              modelName;
    CharString              tpresetName;
    uint32_t                formatVersion;
    uint32_t                loadFlags;
    MetroFSPath             srcFile;
    MetroModelTReplacements treplacements;
};

struct MetroModelSaveParams {
    enum SaveFlags : uint32_t {
        SaveForGameVersion  = 1,
        InlineMeshes        = 2,
        InlineSkeleton      = 8,
    };

    fs::path            dstFile;
    uint32_t            saveFlags = 0;
    MetroGameVersion    gameVersion = MetroGameVersion::Unknown;

    inline bool IsSaveForGameVersion() const {
        return TestBit<uint32_t>(saveFlags, SaveFlags::SaveForGameVersion);
    }
    inline bool IsInlineMeshes() const {
        return TestBit<uint32_t>(saveFlags, SaveFlags::InlineMeshes);
    }
    inline bool IsInlineSkeleton() const {
        return TestBit<uint32_t>(saveFlags, SaveFlags::InlineSkeleton);
    }
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
    Hierarchy2      = 4,    // wtf ???  (seems to be skinned lod mesh hierarchy type)
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
    static const size_t kMaterialStringTexture      = 0;
    static const size_t kMaterialStringShader       = 1;
    static const size_t kMaterialStringGameMaterial = 2;
    static const size_t kMaterialStringSrcMaterial  = 3;
    static const size_t kMaterialStringNumStrings   = 4;

public:
    static uint8_t GetModelVersionFromGameVersion(const MetroGameVersion gameVersion);
    static MetroGameVersion GetGameVersionFromModelVersion(const size_t modelVersion);

public:
    MetroModelBase();
    virtual ~MetroModelBase();

    virtual bool                    Load(MemStream& stream, MetroModelLoadParams& params);
    virtual bool                    Save(MemWriteStream& stream, const MetroModelSaveParams& params);

    void                            SetSourceName(const CharString& srcName);
    const CharString&               GetSourceName() const;

    MetroModelType                  GetModelType() const;
    void                            SetModelType(const MetroModelType type);
    size_t                          GetModelVersion() const;
    void                            SetModelVersion(const size_t version);
    void                            SetModelVersionBasedOnGameVersion(const MetroGameVersion gameVersion);

    virtual uint32_t                GetCheckSum() const;
    virtual size_t                  GetLodCount() const;
    virtual RefPtr<MetroModelBase>  GetLod(const size_t idx) const;

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
    virtual bool                    IsSkinnedHierarchy() const { return false; }
    virtual bool                    IsSoft() const { return false; }

protected:
    virtual void                    ApplyTPresetInternal(const MetroModelTPreset& tpreset);
    virtual void                    ResetTPresetInternal();

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
    CharString                      mMaterialTexture;
    CharString                      mMaterialShader;
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
    virtual bool            Save(MemWriteStream& stream, const MetroModelSaveParams& params) override;

    virtual size_t          GetVerticesMemSize() const override;
    virtual const void*     GetVerticesMemData() const override;
    virtual size_t          GetFacesMemSize() const override;
    virtual const void*     GetFacesMemData() const override;

    virtual void            FreeGeometryMem() override;

    // model creation
    void                    CreateMesh(const size_t numVertices, const size_t numFaces, const size_t numShadowVertices, const size_t numShadowFaces);
    void                    CopyVerticesData(const void* vertices);
    void                    CopyFacesData(const void* faces);
    void                    CopyShadowVerticesData(const void* shadowVertices);
    void                    CopyShadowFacesData(const void* shadowFaces);

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
    virtual bool            Save(MemWriteStream& stream, const MetroModelSaveParams& params) override;

    virtual size_t          GetVerticesMemSize() const override;
    virtual const void*     GetVerticesMemData() const override;
    virtual size_t          GetFacesMemSize() const override;
    virtual const void*     GetFacesMemData() const override;

    virtual void            FreeGeometryMem() override;

    MetroModelSkeleton*     GetParent() const;
    void                    SetParent(MetroModelSkeleton* parent);

    // model creation
    void                    CreateMesh(const size_t numVertices, const size_t numFaces, const float vscale);
    void                    CopyVerticesData(const void* vertices);
    void                    CopyFacesData(const void* faces);
    void                    SetBonesRemapTable(const BytesArray& bonesRemapTable);
    void                    SetBonesOBB(const MyArray<MetroOBB>& bonesOBB);

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

    virtual bool                    Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool                    Save(MemWriteStream& stream, const MetroModelSaveParams& params) override;

    virtual size_t                  GetLodCount() const override;
    virtual RefPtr<MetroModelBase>  GetLod(const size_t idx) const override;

    virtual void                    FreeGeometryMem() override;

    virtual void                    CollectGeomData(MyArray<MetroModelGeomData>& result, const size_t lodIdx = kInvalidValue) const override;

    virtual bool                    IsHierarchy() const override { return true; }
    virtual bool                    IsSkinnedHierarchy() const override { return mType == scast<uint16_t>(MetroModelType::Hierarchy2); }

    size_t                          GetNumTPresets() const;
    MetroModelTPreset&              GetTPreset(const size_t idx);
    const MetroModelTPreset&        GetTPreset(const size_t idx) const;
    void                            AddTPreset(const MetroModelTPreset& tpreset);
    void                            DeleteTPreset(const size_t idx);
    void                            ApplyTPreset(const CharString& tpresetName);
    void                            ResetTPreset();

    size_t                          GetChildrenCount() const;
    RefPtr<MetroModelBase>          GetChild(const size_t idx) const;

    size_t                          GetChildrenRefsCount() const;
    uint32_t                        GetChildRef(const size_t idx) const;

    uint32_t                        GetSkeletonCRC() const;
    void                            SetSkeletonCRC(const uint32_t v);

    virtual void                    AddChild(const RefPtr<MetroModelBase>& child);
    virtual void                    AddLOD(const RefPtr<MetroModelBase>& lod);
    virtual void                    RemoveLODs();

protected:
    void                            LoadTPresets(const StreamChunker& chunker);
    void                            SaveTPresets(MemWriteStream& stream, const uint16_t version);
    virtual void                    ApplyTPresetInternal(const MetroModelTPreset& tpreset) override;
    virtual void                    ResetTPresetInternal() override;

protected:
    using ModelPtr = RefPtr<MetroModelBase>;

    uint32_t                    mSkeletonCRC;   // only skinned hierarchy needs this
    MyArray<ModelPtr>           mChildren;
    MyArray<uint32_t>           mChildrenRefs;
    MyArray<ModelPtr>           mLods;
    MyArray<MetroModelTPreset>  mTPresets;
    MetroModelTReplacements     mTReplacements;
};

// Complete animated model, can consist of multiple Skin models (inline or external *.mesh files)
class MetroModelSkeleton final : public MetroModelHierarchy {
    INHERITED_CLASS(MetroModelHierarchy);

public:
    struct BoneMaterial {
        uint16_t    boneId;
        CharString  material;
    };

    struct BoneMaterialSet {
        CharString              name;
        MyArray<BoneMaterial>   items;
    };

public:
    MetroModelSkeleton();
    virtual ~MetroModelSkeleton();

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream, const MetroModelSaveParams& params) override;

    virtual size_t          GetLodCount() const override;

    virtual bool            IsSkeleton() const override { return true; }

    const RefPtr<MetroSkeleton>& GetSkeleton() const;
    void                    SetSkeleton(RefPtr<MetroSkeleton> skeleton);

    const StringArray&      GetPhysXLinks() const;
    void                    SetPhysXLinks(const StringArray& newLinks);

    void                    AddChildEx(const RefPtr<MetroModelBase>& child);

protected:
    bool                    LoadLodMeshes(MetroModelHierarchy* target, CharString& meshesNames, MetroModelLoadParams& params, const size_t lodIdx);
    bool                    LoadLodMeshes(MetroModelHierarchy* target, MemStream& meshesStream, MetroModelLoadParams& params, const size_t lodIdx);
    bool                    LoadLodMesh(MetroModelHierarchy* target, MemStream& stream, MetroModelLoadParams& params, const size_t lodIdx);

protected:
    using ModelPtr = RefPtr<MetroModelBase>;
    using LodMeshesArr = MyArray<ModelPtr>;

    RefPtr<MetroSkeleton>       mSkeleton;
    MyArray<LodMeshesArr>       mLodMeshes;
    CharString                  mHitPreset;
    MyArray<BoneMaterial>       mGameMaterials;
    MyArray<BoneMaterialSet>    mGameMaterialsPresets;
    MyArray<BoneMaterial>       mMeleeMaterials;
    MyArray<BoneMaterialSet>    mMeleeMaterialsPresets;
    MyArray<BoneMaterial>       mStepMaterials;
    MyArray<BoneMaterialSet>    mStepMaterialsPresets;
    StringArray                 mMotionsFolders;
    StringArray                 mPhysXLinks;
    CharString                  mVoice[3];
    float                       mVoiceParams[3];
};

class MetroModelSoft final : public MetroModelBase {
    INHERITED_CLASS(MetroModelBase);
public:
    MetroModelSoft();
    virtual ~MetroModelSoft();

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream, const MetroModelSaveParams& params) override;

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

