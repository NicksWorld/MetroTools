#include "MetroLevel.h"
#include "MetroContext.h"
#include "MetroBinArchive.h"
#include "MetroModel.h"
#include "reflection/MetroReflection.h"

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

enum SectorDescriptionChunk : size_t {
    SDC_Header      = 0x00000001,
    SDC_Materials   = 0x00000002,
    SDC_Sections    = 0x00000003,
    SDC_Instances   = 0x00000008,
};

enum SectorSectionChunk : size_t {
    SSC_Desc        = 0x00000015,
    SSC_Materials   = 0x00000002,
    SSC_MeshHeader  = 0x00000001,
};

enum SectorGeomChunks : size_t {
    SGC_Version     = 0x00000001,
    SGC_Vertices    = 0x00000009,
    SGC_Indices     = 0x0000000A,
    SGC_Checksum    = 0x0000000B, // ???
};


static const size_t kLevelVersionArktika    = 17;
static const size_t kLevelVersionExodus     = 19;

void MetroTerrain::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, version);     // 2 - Arktika.1, 3 - Exodus
    if (version == 3) {
        METRO_SERIALIZE_MEMBER(s, has_terrain);
        if (has_terrain) {
            METRO_SERIALIZE_MEMBER(s, hmap);
            METRO_SERIALIZE_MEMBER_CHOOSE(s, diffuse);
            METRO_SERIALIZE_MEMBER_CHOOSE(s, shader);
            METRO_SERIALIZE_MEMBER(s, dim_min);
            METRO_SERIALIZE_MEMBER(s, dim_size);
            METRO_SERIALIZE_MEMBER(s, hmap_width);
            METRO_SERIALIZE_MEMBER(s, hmap_height);
            //mtl_mask_names
        }
    } else {
        //#TODO_SK: implement me!!!
    }
}

void MetroEntitiesParams::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, version);
}

void MetroLevelEntity::Serialize(MetroReflectionStream& s) {
    UObjectInitData initData;
    s >> initData;

    this->uobject = MetroEntityFactory::Get().CreateUObject(initData);
    if (this->uobject) {
        s >> (*this->uobject);

        mat4 pose = MatFromPose(this->uobject->pose);
        vec3 pos, scale;
        quat rot;
        MatDecompose(pose, pos, scale, rot);

        LogPrintF(LogLevel::Info, "%s, id = %d, parent_id = %d, name = %s, visual = %s, pos = (%f, %f, %f)", this->uobject->cls.c_str(),
            this->uobject->initData.id, this->uobject->initData.parent_id, this->uobject->name.empty() ? "" : this->uobject->name.c_str(),
            this->uobject->visual.empty() ? "none" : this->uobject->visual.c_str(),
            pos.x, pos.y, pos.z);
        if (s.GetRemains() != 0) {
            LogPrintF(LogLevel::Warning, "UObject, remains [%zu] bytes", s.GetRemains());
        }
    }
}


MetroLevel::MetroLevel()
    : entities_params{}
    , mTerrain{}
{
}
MetroLevel::~MetroLevel() {
}

bool MetroLevel::LoadFromFileHandle(const MetroFSPath& file) {
    bool result = false;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    CharString filePath = mfs.GetFullPath(file);

    CharString::size_type lastSlashPos = filePath.find_last_of('\\');
    CharString levelFolder = filePath.substr(0, lastSlashPos + 1);

    if (MetroContext::Get().GetGameVersion() >= MetroGameVersion::Arktika1) {
        this->LoadGeoModern(levelFolder);
    } else {
        this->LoadGeoLegacy(levelFolder);
    }

    this->LoadTerrain(levelFolder);

    this->LoadBin(file);

    result = !mSectors.empty() || !entities.empty();

    return result;
}

// materials
size_t MetroLevel::GetNumMaterials() const {
    return mMaterials.size();
}

const LevelMaterial& MetroLevel::GetMaterial(const size_t idx) const {
    return mMaterials[idx];
}

// level geo
size_t MetroLevel::GetNumSectors() const {
    return mSectors.size();
}

const LevelSector& MetroLevel::GetSector(const size_t idx) const {
    return mSectors[idx];
}

// terrain
bool MetroLevel::HasTerrain() const {
    return mTerrain.has_terrain;
}

const CharString& MetroLevel::GetTerrainHMapName() const {
    return mTerrain.hmap;
}

const CharString& MetroLevel::GetTerrainLMapName() const {
    return mTerrain._lmapFullName;
}

const CharString& MetroLevel::GetTerrainDiffuseName() const {
    return mTerrain.diffuse;
}

size_t MetroLevel::GetTerrainHMapWidth() const {
    return mTerrain.hmap_width;
}

size_t MetroLevel::GetTerrainHMapHeight() const {
    return mTerrain.hmap_height;
}

const vec3& MetroLevel::GetTerrainDimMin() const {
    return mTerrain.dim_min;
}

const vec3& MetroLevel::GetTerrainDimSize() const {
    return mTerrain.dim_size;
}

// objects
size_t MetroLevel::GetNumEntities() const {
    return entities.size();
}

const CharString& MetroLevel::GetEntityName(const size_t idx) const {
    return entities[idx].uobject->name;
}

const CharString& MetroLevel::GetEntityClassName(const size_t idx) const {
    const TypedString& ts = MetroContext::Get().GetTypedStrings().GetString(entities[idx].uobject->initData.cls);
    return ts.str;
}

const CharString& MetroLevel::GetEntityVisual(const size_t idx) const {
    return entities[idx].uobject->visual;
}

const CharString& MetroLevel::GetEntityAttachBone(const size_t idx) const {
    return entities[idx].uobject->initData.att_bone_id;
}

const pose_43T& MetroLevel::GetEntityAttachTransform(const size_t idx) const {
    return entities[idx].uobject->initData.att_offset;
}

const pose_43& MetroLevel::GetEntityTransform(const size_t idx) const {
    return entities[idx].uobject->pose;
}

size_t MetroLevel::GetEntityID(const size_t idx) const {
    return entities[idx].uobject->initData.id;
}

size_t MetroLevel::GetEntityParentID(const size_t idx) const {
    return entities[idx].uobject->initData.parent_id;
}



void MetroLevel::LoadGeoModern(const CharString& levelFolder) {
    CharString levelStaticFolder = levelFolder + "static\\";
    CharString sectorsListFile = levelStaticFolder + "level.lightmaps";

    StringArray sectorsList = this->ReadSectorsList(sectorsListFile);
    mSectors.reserve(sectorsList.size());

    for (const CharString& sname : sectorsList) {
        this->ReadSector(sname, levelStaticFolder);
    }
}

void MetroLevel::LoadGeoLegacy(const CharString& levelFolder) {
    mSectors.reserve(1);
    this->ReadSector("level", levelFolder);
}

StringArray MetroLevel::ReadSectorsList(const CharString& path) {
    StringArray result;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
    MemStream stream = mfs.OpenFileFromPath(path);
    if (stream.Good()) {
        /*const size_t version = stream.ReadU32();*/ stream.SkipBytes(sizeof(uint32_t));
        /*if (version == 1)*/ {
            const size_t numSectors = stream.ReadU32();

            result.resize(numSectors);
            for (CharString& s : result) {
                s = stream.ReadStringZ();
            }
        }
    }

    return result;
}

void MetroLevel::ReadSector(const CharString& sectorName, const CharString& folder) {
    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    LevelSector sector;
    sector.name = sectorName;

    CharString sectorDescPath = folder + sectorName;
    CharString sectorGeomPath = folder + sectorName + ".geom_pc";

    bool isBEData = false;

    MetroFSPath sectorDescFile = mfs.FindFile(sectorDescPath);
    MetroFSPath sectorGeomFile = mfs.FindFile(sectorGeomPath);
    if (!sectorGeomFile.IsValid()) {
        sectorGeomPath = folder + sectorName + ".geom_xbox";
        sectorGeomFile = mfs.FindFile(sectorGeomPath);
        isBEData = true;
    }

    if (sectorDescFile.IsValid() && sectorGeomFile.IsValid()) {
        mSectors.emplace_back(sector);

        LevelSector& newSector = mSectors.back();

        // read sector description
        {
            size_t version = 0, flags = 0;

            MemStream stream = mfs.OpenFileStream(sectorDescFile);
            while (!stream.Ended()) {
                const size_t chunkIdx = stream.ReadU32();
                const size_t chunkSize = stream.ReadU32();
                const size_t chunkEnd = stream.GetCursor() + chunkSize;

                switch (chunkIdx) {
                    case SDC_Header: {
                        version = stream.ReadU16();
                        flags = stream.ReadU16();
                    } break;

                    case SDC_Materials: {
                        const size_t numMaterials = stream.ReadU16();
                        mMaterials.resize(numMaterials);
                        for (auto& mat : mMaterials) {
                            mat.shader = stream.ReadStringZ();
                            mat.texture = stream.ReadStringZ();
                            mat.material = stream.ReadStringZ();
                            mat.flags = stream.ReadU32();
                        }
                    } break;

                    case SDC_Sections: {
                        MemStream subStream = stream.Substream(chunkSize);
                        size_t idx = 0;
                        while (!subStream.Ended()) {
                            const size_t sectionIdx = subStream.ReadU32();
                            const size_t sectionSize = subStream.ReadU32();
                            const size_t sectionEnd = subStream.GetCursor() + sectionSize;

                            assert(idx == sectionIdx);
                            ++idx;

                            MemStream mdlStream = subStream.Substream(sectionSize);

                            MetroModelLoadParams params = {
                                kEmptyString,
                                kEmptyString,
                                scast<uint32_t>(version),
                                MetroModelLoadParams::LoadEverything,
                                MetroFSPath(MetroFSPath::Invalid)
                            };
                            RefPtr<MetroModelBase> mdl = MetroModelFactory::CreateModelFromStream(mdlStream, params);
                            if (mdl) {
                                newSector.superStaticMeshes.emplace_back(mdl);
                            }

                            subStream.SetCursor(sectionEnd);
                        }
                    } break;

                    case SDC_Instances: {
                        MemStream subStream = stream.Substream(chunkSize);
                        size_t idx = 0;
                        while (!subStream.Ended()) {
                            const size_t sectionIdx = subStream.ReadU32();
                            const size_t sectionSize = subStream.ReadU32();
                            const size_t sectionEnd = subStream.GetCursor() + sectionSize;

                            assert(idx == sectionIdx);
                            ++idx;

                            const size_t instanceChunkIdx = subStream.ReadU32();
                            const size_t instanceChunkSize = subStream.ReadU32();
                            assert(instanceChunkIdx == 2);
                            assert(instanceChunkSize == 4);

                            newSector.superStaticInstances.push_back(subStream.ReadU32());

                            if (version >= kLevelVersionExodus) {
                                newSector.name = subStream.ReadStringZ();
                                newSector.lmapScale = subStream.ReadF32();
                            }

                            subStream.SetCursor(sectionEnd);
                        }
                    } break;
                }

                stream.SetCursor(chunkEnd);
            }
        }

        // read sector geometry
        {
            size_t version = 0, flags = 0, checksum = 0;

            MemStream stream = mfs.OpenFileStream(sectorGeomFile);
            while (!stream.Ended()) {
                const size_t chunkIdx = stream.ReadU32();
                const size_t chunkSize = stream.ReadU32();
                const size_t chunkEnd = stream.GetCursor() + chunkSize;

                switch (chunkIdx) {
                    case SGC_Version: {
                        version = stream.ReadU16();
                        flags = stream.ReadU16();
                    } break;

                    case SGC_Checksum: {
                        checksum = stream.ReadU32();
                    } break;

                    case SGC_Vertices: {
                        size_t totalVertices = 0;
                        if (version >= kLevelVersionExodus) {
                            //const size_t numVertices = stream.ReadU32();
                            //const size_t numShadowVertices = stream.ReadU32();
                            stream.SkipBytes(8);

                            //totalVertices = numVertices + ((numShadowVertices + 1) / 2);
                            totalVertices = (chunkSize - 8) / sizeof(VertexLevel);
                        } else {
                            totalVertices = chunkSize / sizeof(VertexLevel);
                        }

                        newSector.vertices.resize(totalVertices);
                        stream.ReadToBuffer(newSector.vertices.data(), totalVertices * sizeof(VertexLevel));

                        if (isBEData) {
                            std::transform(newSector.vertices.begin(), newSector.vertices.end(), newSector.vertices.begin(), [](const VertexLevel& v)->VertexLevel {
                                VertexLevel result;
                                *rcast<uint32_t*>(&result.pos.x) = EndianSwapBytes(*rcast<const uint32_t*>(&v.pos.x));
                                *rcast<uint32_t*>(&result.pos.y) = EndianSwapBytes(*rcast<const uint32_t*>(&v.pos.y));
                                *rcast<uint32_t*>(&result.pos.z) = EndianSwapBytes(*rcast<const uint32_t*>(&v.pos.z));
                                result.normal = EndianSwapBytes(v.normal);
                                result.aux0 = EndianSwapBytes(v.aux0);
                                result.aux1 = EndianSwapBytes(v.aux1);
                                result.uv0[0] = EndianSwapBytes(v.uv0[0]);
                                result.uv0[1] = EndianSwapBytes(v.uv0[1]);
                                result.uv1[0] = EndianSwapBytes(v.uv1[0]);
                                result.uv1[1] = EndianSwapBytes(v.uv1[1]);
                                return result;
                            });
                        }
                    } break;

                    case SGC_Indices: {
                        size_t totalIndices = 0;
                        if (version >= kLevelVersionExodus) {
                            //const size_t numIndices = stream.ReadU32();
                            //const size_t numShadowIndices = stream.ReadU32();
                            stream.SkipBytes(8);

                            //totalIndices = numIndices + numShadowIndices;
                            totalIndices = (chunkSize - 8) / sizeof(uint16_t);
                        } else {
                            totalIndices = chunkSize / sizeof(uint16_t);
                        }

                        newSector.indices.resize(totalIndices);
                        stream.ReadToBuffer(newSector.indices.data(), totalIndices * sizeof(uint16_t));

                        if (isBEData) {
                            std::transform(newSector.indices.begin(), newSector.indices.end(), newSector.indices.begin(), [](const uint16_t& v)->uint16_t {
                                return EndianSwapBytes(v);
                            });
                        }
                    } break;
                }

                stream.SetCursor(chunkEnd);
            }
        }
    }
}

void MetroLevel::LoadTerrain(const CharString& levelFolder) {
    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    mTerrain.has_terrain = false;

    CharString terrainDescPath = levelFolder + "level.terrain";

    MemStream stream = mfs.OpenFileFromPath(terrainDescPath);
    if (stream.Good()) {
        MetroBinArchive bin(kEmptyString, stream, MetroBinArchive::kHeaderNotExist);
        StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();
        if (reader) {
            (*reader) >> mTerrain;

            if (mTerrain.has_terrain) {
                if (mTerrain.hmap == "$hmap") {
                    mTerrain._hmapIsImplicit = true;
                    mTerrain.hmap = levelFolder + "level.terrain_hmap";
                }

                mTerrain._lmapFullName = levelFolder + "terrain\\level_terrain.lmap_pc";
            }
        }
    }
}

static_assert(0x6C76656Cu == 'lvel');

void MetroLevel::LoadBin(const MetroFSPath& file) {
    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
    MemStream stream = mfs.OpenFileStream(file);
    if (stream.Good()) {
        const uint32_t magic = stream.ReadU32();
        if (magic == 'lvel') {
            stream.SetCursor(0);

            MetroBinArchive bin(kEmptyString, stream, 4);
            StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();
            if (reader) {
                MetroReflectionStream* startupReader = reader->OpenSection("startup");
                if (startupReader) {
                    //#TODO_SK:
                    reader->CloseSection(startupReader);
                }

                METRO_SERIALIZE_STRUCT_MEMBER(*reader, entities_params);
                reader->SetUserData(entities_params.version);
                if (entities_params.version >= 40) {
                    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*reader, entities);
                }
            }
        }
    }
}
