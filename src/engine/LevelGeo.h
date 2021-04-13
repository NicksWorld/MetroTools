#pragma once
#include "mycommon.h"
#include "mymath.h"

#include "engine/Surface.h"

class MetroLevel;
class MetroModelBase;

struct LevelSector;

struct ID3D11Device;
struct ID3D11Buffer;

namespace u4a {

struct LevelGeoSection {
    uint32_t    numVertices;
    uint32_t    numIndices;
    uint32_t    numShadowVertices;
    uint32_t    numShadowIndices;
    uint32_t    vbOffset;
    uint32_t    ibOffset;
    uint32_t    shadowVBOffset;
    uint32_t    shadowIBOffset;
    size_t      surfaceIdx;
};

struct LevelGeoSector {
    AABBox                      bbox;
    BSphere                     bsphere;
    MyArray<LevelGeoSection>    sections;

    ID3D11Buffer*               vertexBuffer = nullptr;
    ID3D11Buffer*               indexBuffer = nullptr;
};


struct TerrainChunk {
    size_t  numIndices;
    AABBox  bbox;
};

class LevelGeo {
public:
    LevelGeo();
    ~LevelGeo();

    size_t                  GetNumSectors() const;
    const LevelGeoSector&   GetSector(const size_t idx) const;
    const Surface&          GetSurface(const size_t idx) const;

    const vec3&             GetTerrainMin() const;
    const vec3&             GetTerrainDim() const;
    ID3D11Buffer*           GetTerrainVB() const;
    ID3D11Buffer*           GetTerrainIB() const;
    Texture*                GetTerrainDiffuse() const;
    Texture*                GetTerrainNormalmap() const;
    Texture*                GetTerrainMask() const;
    Texture*                GetTerrainDet(const size_t idx) const;
    Texture*                GetTerrainLMap() const;
    Texture*                GetTerrainHMap() const;
    size_t                  GetTerrainHMapWidth() const;
    size_t                  GetTerrainHMapHeight() const;
    size_t                  GetTerrainNumChunks() const;
    size_t                  GetTerrainNumChunksX() const;
    size_t                  GetTerrainNumChunksY() const;
    const TerrainChunk&     GetTerrainChunk(const size_t idx) const;

    bool                    Create(const MetroLevel* level);
    void                    Destroy();

private:
    void                    AddSectorSuperStaticMesh(LevelGeoSector& sector, const RefPtr<MetroModelBase>& ssm, const LevelSector& ls);
    bool                    CreateTerrain();

private:
    // level geo
    MyArray<LevelGeoSector> mSectors;
    MyArray<Surface>        mSurfaces;

    // terrain
    vec3                    mTerrainMin;
    vec3                    mTerrainDim;
    Texture*                mTerrainDiffuse;
    Texture*                mTerrainNormalmap;
    Texture*                mTerrainMask;
    Texture*                mTerrainDet0;
    Texture*                mTerrainDet1;
    Texture*                mTerrainDet2;
    Texture*                mTerrainDet3;
    Texture*                mTerrainLMap;
    Texture*                mTerrainHMap;
    size_t                  mTerrainHMapWidth;
    size_t                  mTerrainHMapHeight;
    ID3D11Buffer*           mTerrainVB;
    ID3D11Buffer*           mTerrainIB;
    size_t                  mTerrainNumChunksX;
    size_t                  mTerrainNumChunksY;
    MyArray<TerrainChunk>   mTerrainChunks;
};

} // namespace u4a
