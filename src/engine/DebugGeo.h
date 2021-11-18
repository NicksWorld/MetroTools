#pragma once
#include "EngineTypes.h"

struct ID3D11Device;
struct ID3D11Buffer;

namespace u4a {

struct DebugGeoSection {
    uint32_t    numVertices;
    uint32_t    numIndices;
    uint32_t    vbOffset;
    uint32_t    ibOffset;
};

class DebugGeo {
    struct DebugInputSection {
        MyArray<VertexDebug>    vertices;
        MyArray<uint16_t>       indices;
    };

public:
    DebugGeo();
    ~DebugGeo();

    void                        AddSection(const vec3* vertices, const size_t numVertices, const uint16_t* indices, const size_t numIndices);
    bool                        Create();
    void                        Destroy();

    size_t                      GetNumSections() const;
    const DebugGeoSection&      GetSection(const size_t idx) const;

    ID3D11Buffer*               GetVertexBuffer() const;
    ID3D11Buffer*               GetIndexBuffer() const;


private:
    MyArray<DebugInputSection>  mInputSections; // transient
    MyArray<DebugGeoSection>    mSections;
    ID3D11Buffer*               mVertexBuffer;
    ID3D11Buffer*               mIndexBuffer;
};

} // namespace u4a
