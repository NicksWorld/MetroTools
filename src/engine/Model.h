#pragma once
#include "mycommon.h"
#include "mymath.h"

#include "engine/Surface.h"

class MetroModel;
class MetroModelBase;
class MetroSkeleton;

struct ID3D11Device;
struct ID3D11Buffer;

namespace u4a {

struct MeshSection {
    uint32_t    numVertices;
    uint32_t    numIndices;
    uint32_t    numShadowVertices;
    uint32_t    numShadowIndices;
    uint32_t    vbOffset;
    uint32_t    ibOffset;
    uint32_t    shadowVBOffset;
    uint32_t    shadowIBOffset;
    float       vscale;
    float       alphaCut;
};

class Model {
public:
    enum class Type : size_t {
        Static  = 0,
        Skinned = 1
    };

public:
    Model();
    virtual ~Model();

    const Type              GetType() const;

    const AABBox&           GetBBox() const;
    const BSphere&          GetBSphere() const;

    const size_t            GetNumSections() const;
    const MeshSection&      GetSection(const size_t idx) const;
    const Surface&          GetSurface(const size_t idx) const;

    const mat4              GetBoneTransform(const CharString& boneName) const;

    bool                    HasTransparency() const;
    bool                    HasTranslucency() const;

    bool                    Create(const MetroModel* mdl);
    bool                    CreateNew(const MetroModelBase* mdl);
    void                    Destroy();

    ID3D11Buffer*           GetVertexBuffer() const;
    ID3D11Buffer*           GetIndexBuffer() const;

    const MetroSkeleton*    GetSkeleton() const;

    void                    SetSource(const CharString& src) { mSource = src; }

protected:
    Model::Type             mType;
    AABBox                  mBBox;
    BSphere                 mBSphere;
    MyArray<MeshSection>    mSections;

    ID3D11Buffer*           mVertexBuffer;
    ID3D11Buffer*           mIndexBuffer;

    MyArray<Surface>        mSurfaces;

    MetroSkeleton*          mSkeleton;

    CharString              mSource;
};

} // namespace u4a
