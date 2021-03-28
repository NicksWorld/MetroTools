#pragma once
#include "MetroTypes.h"

class MetroSkeleton;
class MetroMotion;

struct MetroModelLoadParams {
    enum LoadFlags : uint32_t {
        LoadGeometry    = 1,
        LoadCollision   = 2,
        LoadSkeleton    = 4,

        LoadEverything  = ~0u
    };

    CharString  modelName;
    CharString  tpresetName;
    uint32_t    formatVersion;
    uint32_t    loadFlags;
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
};

class MetroModelBase;
class MetroModelStd;
class MetroModelSkin;
class MetroModelHierarchy;
class MetroModelSkeleton;

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
    const BSphere&                  GetBSphere() const;

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

    virtual void                    CollectGeomData(MyArray<MetroModelGeomData>& result) const;

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
class MetroModelStd : public MetroModelBase {
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
class MetroModelSkin : public MetroModelBase {
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
    BytesArray              mBonesRemap;
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

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream) override;

    virtual void            FreeGeometryMem() override;

    virtual void            CollectGeomData(MyArray<MetroModelGeomData>& result) const override;

    size_t                  GetChildrenCount() const;
    RefPtr<MetroModelBase>  GetChild(const size_t idx) const;

    void                    AddChild(const RefPtr<MetroModelBase>& child);

protected:
    using ModelPtr = RefPtr<MetroModelBase>;

    MyArray<ModelPtr>       mChildren;
    MyArray<ModelPtr>       mLods;
};

// Complete animated model, can consist of multiple Skin models (inline or external *.mesh files)
class MetroModelSkeleton : public MetroModelHierarchy {
    INHERITED_CLASS(MetroModelHierarchy);
public:
    MetroModelSkeleton();
    virtual ~MetroModelSkeleton();

    virtual bool            Load(MemStream& stream, MetroModelLoadParams& params) override;
    virtual bool            Save(MemWriteStream& stream) override;

    RefPtr<MetroSkeleton>   GetSkeleton() const;

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


class MetroModelFactory {
    MetroModelFactory() = delete;
    MetroModelFactory(const MetroModelFactory&) = delete;
    MetroModelFactory(MetroModelFactory&&) = delete;

public:
    static RefPtr<MetroModelBase>   CreateModelFromType(const MetroModelType type);
    static RefPtr<MetroModelBase>   CreateModelFromStream(MemStream& stream, const uint32_t loadFlags);
};


class MetroModel {
    struct TexturePreset {
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

public:
    static const size_t kMetroModelMaxLods          = 2;

public:
    MetroModel();
    ~MetroModel();

    bool                    LoadFromName(const CharString& name, const bool needAnimations);
    bool                    LoadFromData(MemStream& stream, const size_t fileIdx, const bool needAnimations = true);

    bool                    IsAnimated() const;
    const AABBox&           GetBBox() const;
    const BSphere&          GetBSphere() const;
    size_t                  GetNumMeshes() const;
    const MetroMesh*        GetMesh(const size_t idx) const;
    MyArray<MetroVertex>    MakeCommonVertices(const size_t meshIdx) const;

    const CharString&       GetSkeletonPath() const;
    const MetroSkeleton*    GetSkeleton() const;
    MetroModel*             GetLodModel(const size_t lodId) const;
    size_t                  GetNumMotions() const;
    CharString              GetMotionName(const size_t idx) const;
    const CharString&       GetMotionPath(const size_t idx) const;
    float                   GetMotionDuration(const size_t idx) const;
    const MetroMotion*      GetMotion(const size_t idx);

    const CharString&       GetTextureReplacement(const HashString& name) const;
    const CharString&       GetComment() const;

private:
    void                    ReadSubChunks(MemStream& stream);
    void                    LoadLinkedMeshes(const StringArray& links);
    void                    ReplaceTextures();
    void                    SetTPreset(const CharString& tpreset);
    void                    LoadMotions();

private:
    struct MotionInfo {
        MyHandle        file;
        size_t          numFrames;
        CharString      path;
        MetroMotion*    motion;
    };

    size_t                          mVersion;
    AABBox                          mBBox;
    BSphere                         mBSphere;
    size_t                          mType;
    MyArray<MetroMesh*>             mMeshes;
    MetroModel*                     mLodModels[kMetroModelMaxLods];
    CharString                      mSkeletonPath;
    MetroSkeleton*                  mSkeleton;
    MyArray<MotionInfo>             mMotions;
    MyDict<HashString, CharString>  mTextureReplacements;
    MyArray<TexturePreset>          mTexturePresets;
    CharString                      mComment;

    // these are temp pointers, invalid after loading
    MetroMesh*                      mCurrentMesh;
    size_t                          mThisFileIdx;
};
