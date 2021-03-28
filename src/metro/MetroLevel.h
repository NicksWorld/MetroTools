#pragma once
#include "MetroTypes.h"
#include "entities/MetroEntityFactory.h"

class MetroTexture;
class MetroReflectionStream;

PACKED_STRUCT_BEGIN
struct SectorSectionDesc {  // size = 28
    uint32_t    vertexType;
    uint32_t    vbOffset;
    uint32_t    numVertices;
    uint32_t    numShadowVertices;
    uint32_t    ibOffset;
    uint32_t    numIndices;
    uint32_t    numShadowIndices;
} PACKED_STRUCT_END;

struct SectorSection {
    AABBox              bbox;
    BSphere             bsphere;
    float               texelDensity;
    SectorSectionDesc   desc;
    CharString          textureName;
    CharString          shaderName;
    CharString          materialName;
};

struct LevelSector {
    CharString              name;
    MyArray<SectorSection>  sections;
    MyArray<VertexLevel>    vertices;
    MyArray<uint16_t>       indices;
    MetroTexture*           lmap;
};

struct MetroTerrain {
    // serialized props
    uint32_t    version;
    bool        has_terrain;
    CharString  hmap;
    CharString  diffuse;            // choose
    CharString  shader;             // choose
    vec3        dim_min;
    vec3        dim_size;
    uint32_t    hmap_width;
    uint32_t    hmap_height;
    StringArray mtl_mask_names;     // str_array32
    //BytesArray  td_headers;         // u8_array
    //uint32_t    td_params_size;
    //BytesArray  td_params;          // u8_array

    // transient params
    bool        _hmapIsImplicit; // $hmap
    CharString  _lmapFullName;

    void Serialize(MetroReflectionStream& s);
};

struct MetroEntitiesParams {
    uint16_t    version;

    void Serialize(MetroReflectionStream& s);
};

struct MetroLevelEntity {
    UObjectPtr  uobject;

    void Serialize(MetroReflectionStream& s);
};

class MetroLevel {
public:
    MetroLevel();
    ~MetroLevel();

    bool                        LoadFromFileHandle(const MyHandle file);

    // level geo
    size_t                      GetNumSectors() const;
    const LevelSector&          GetSector(const size_t idx) const;

    // terrain
    bool                        HasTerrain() const;
    const CharString&           GetTerrainHMapName() const;
    const CharString&           GetTerrainLMapName() const;
    const CharString&           GetTerrainDiffuseName() const;
    size_t                      GetTerrainHMapWidth() const;
    size_t                      GetTerrainHMapHeight() const;
    const vec3&                 GetTerrainDimMin() const;
    const vec3&                 GetTerrainDimSize() const;

    // objects
    size_t                      GetNumEntities() const;
    const CharString&           GetEntityName(const size_t idx) const;
    const CharString&           GetEntityClassName(const size_t idx) const;
    const CharString&           GetEntityVisual(const size_t idx) const;
    const CharString&           GetEntityAttachBone(const size_t idx) const;
    const pose_43T&             GetEntityAttachTransform(const size_t idx) const;
    const pose_43&              GetEntityTransform(const size_t idx) const;
    size_t                      GetEntityID(const size_t idx) const;
    size_t                      GetEntityParentID(const size_t idx) const;

private:
    void                        LoadGeoModern(const CharString& levelFolder);
    void                        LoadGeoLegacy(const CharString& levelFolder);

    StringArray                 ReadSectorsList(const CharString& path);
    void                        ReadSector(const CharString& sectorName, const CharString& folder);
    void                        ReadSectorSection(MemStream& stream, SectorSection& section, const size_t version);

    void                        LoadTerrain(const CharString& levelFolder);

    void                        LoadBin(const MyHandle file);

private:
    MyArray<LevelSector>        mSectors;
    MetroEntitiesParams         entities_params;
    MyArray<MetroLevelEntity>   entities;
    MetroTerrain                mTerrain;
};
