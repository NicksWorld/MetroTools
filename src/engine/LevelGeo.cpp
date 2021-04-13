#include "LevelGeo.h"

#include "engine/Renderer.h"
#include "engine/ResourcesManager.h"

#include "metro/MetroLevel.h"
#include "metro/MetroModel.h"
#include "metro/MetroContext.h"

#define D3D11_NO_HELPERS
#include <d3d11.h>


namespace u4a {

LevelGeo::LevelGeo()
    : mTerrainMin{}
    , mTerrainDim{}
    , mTerrainDiffuse(nullptr)
    , mTerrainNormalmap(nullptr)
    , mTerrainMask(nullptr)
    , mTerrainDet0(nullptr)
    , mTerrainDet1(nullptr)
    , mTerrainDet2(nullptr)
    , mTerrainDet3(nullptr)
    , mTerrainLMap(nullptr)
    , mTerrainHMap(nullptr)
    , mTerrainHMapWidth(0)
    , mTerrainHMapHeight(0)
    , mTerrainVB(nullptr)
    , mTerrainIB(nullptr)
    , mTerrainNumChunksX(0)
    , mTerrainNumChunksY(0)
{
}
LevelGeo::~LevelGeo() {
    this->Destroy();
}

size_t LevelGeo::GetNumSectors() const {
    return mSectors.size();
}

const LevelGeoSector& LevelGeo::GetSector(const size_t idx) const {
    return mSectors[idx];
}

const Surface& LevelGeo::GetSurface(const size_t idx) const {
    return mSurfaces[idx];
}

const vec3& LevelGeo::GetTerrainMin() const {
    return mTerrainMin;
}

const vec3& LevelGeo::GetTerrainDim() const {
    return mTerrainDim;
}

ID3D11Buffer* LevelGeo::GetTerrainVB() const {
    return mTerrainVB;
}

ID3D11Buffer* LevelGeo::GetTerrainIB() const {
    return mTerrainIB;
}

Texture* LevelGeo::GetTerrainDiffuse() const {
    return mTerrainDiffuse;
}

Texture* LevelGeo::GetTerrainNormalmap() const {
    return mTerrainNormalmap;
}

Texture* LevelGeo::GetTerrainMask() const {
    return mTerrainMask;
}

Texture* LevelGeo::GetTerrainDet(const size_t idx) const {
    Texture* result = nullptr;

    switch (idx) {
        case 0: result = mTerrainDet0; break;
        case 1: result = mTerrainDet1; break;
        case 2: result = mTerrainDet2; break;
        case 3: result = mTerrainDet3; break;
    }

    return result;
}

Texture* LevelGeo::GetTerrainLMap() const {
    return mTerrainLMap;
}

Texture* LevelGeo::GetTerrainHMap() const {
    return mTerrainHMap;
}

size_t LevelGeo::GetTerrainHMapWidth() const {
    return mTerrainHMapWidth;
}

size_t LevelGeo::GetTerrainHMapHeight() const {
    return mTerrainHMapHeight;
}

size_t LevelGeo::GetTerrainNumChunks() const {
    return mTerrainChunks.size();
}

size_t LevelGeo::GetTerrainNumChunksX() const {
    return mTerrainNumChunksX;
}

size_t LevelGeo::GetTerrainNumChunksY() const {
    return mTerrainNumChunksY;
}

const TerrainChunk& LevelGeo::GetTerrainChunk(const size_t idx) const {
    return mTerrainChunks[idx];
}

bool LevelGeo::Create(const MetroLevel* level) {
    bool result = false;

    const size_t numSectors = level->GetNumSectors();
    mSectors.resize(numSectors);

    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA subData = {};
    HRESULT hr;

    ID3D11Device* device = Renderer::Get().GetDevice();

    for (size_t i = 0; i < numSectors; ++i) {
        const LevelSector& ls = level->GetSector(i);
        LevelGeoSector& sector = mSectors[i];

        //vb
        desc.ByteWidth = scast<UINT>(ls.vertices.size() * sizeof(VertexLevel));
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        subData.pSysMem = ls.vertices.data();
        hr = device->CreateBuffer(&desc, &subData, &sector.vertexBuffer);
        if (FAILED(hr)) {
            return false;
        }

        //ib
        desc.ByteWidth = scast<UINT>(ls.indices.size() * sizeof(uint16_t));
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        subData.pSysMem = ls.indices.data();
        hr = device->CreateBuffer(&desc, &subData, &sector.indexBuffer);
        if (FAILED(hr)) {
            return false;
        }

        sector.bbox.Reset();

        const size_t numSuperStaticMeshes = ls.superStaticMeshes.size();
        sector.sections.reserve(numSuperStaticMeshes);

        for (const uint32_t i : ls.superStaticInstances) {
            const RefPtr<MetroModelBase>& ssm = ls.superStaticMeshes[i];
            this->AddSectorSuperStaticMesh(sector, ssm, ls);
        }
    }

    // build terrain geometry
    if (level->HasTerrain()) {
        const CharString& hmapPath = level->GetTerrainHMapName();
        mTerrainHMapWidth = level->GetTerrainHMapWidth();
        mTerrainHMapHeight = level->GetTerrainHMapHeight();
        mTerrainMin = level->GetTerrainDimMin();
        mTerrainDim = level->GetTerrainDimSize();

        mTerrainHMap = ResourcesManager::Get().GetHMapTexture(level->GetTerrainHMapName(), mTerrainHMapWidth, mTerrainHMapHeight);
        if (mTerrainHMap && this->CreateTerrain()) {
            HashString diffuseName = level->GetTerrainDiffuseName();
            const CharString& bumpName = MetroContext::Get().GetTexturesDB().GetBumpName(diffuseName);
            const CharString& aux0Name = MetroContext::Get().GetTexturesDB().GetAuxName(diffuseName, 0);
            const CharString& detName = MetroContext::Get().GetTexturesDB().GetDetName(diffuseName);
            StringArray detNames = StrSplit(detName, ',');

            mTerrainDiffuse = ResourcesManager::Get().GetSimpleTexture(diffuseName, false);
            mTerrainNormalmap = ResourcesManager::Get().GetSimpleTexture(aux0Name, true);
            mTerrainMask = ResourcesManager::Get().GetSimpleTexture(bumpName, true);
            mTerrainDet0 = ResourcesManager::Get().GetSimpleTexture(detNames.size() > 0 ? detNames[0] : kEmptyHashString, false);
            mTerrainDet1 = ResourcesManager::Get().GetSimpleTexture(detNames.size() > 1 ? detNames[1] : kEmptyHashString, false);
            mTerrainDet2 = ResourcesManager::Get().GetSimpleTexture(detNames.size() > 2 ? detNames[2] : kEmptyHashString, false);
            mTerrainDet3 = ResourcesManager::Get().GetSimpleTexture(detNames.size() > 3 ? detNames[3] : kEmptyHashString, false);

            mTerrainLMap = ResourcesManager::Get().GetLMapTexture(level->GetTerrainLMapName());
        }
    }

    result = true;

    return result;
}

void LevelGeo::Destroy() {
    for (auto& s : mSectors) {
        MySafeRelease(s.vertexBuffer);
        MySafeRelease(s.indexBuffer);
    }

    mSectors.clear();
    mSurfaces.clear();

    MySafeRelease(mTerrainVB);
    MySafeRelease(mTerrainIB);
}



void LevelGeo::AddSectorSuperStaticMesh(LevelGeoSector& sector, const RefPtr<MetroModelBase>& ssm, const LevelSector& ls) {
    if (ssm->IsHierarchy()) {
        RefPtr<MetroModelHierarchy> hierarchyL = SCastRefPtr<MetroModelHierarchy>(ssm);
        const size_t numChildrenL = hierarchyL->GetChildrenRefsCount();
        for (size_t k = 0; k < numChildrenL; ++k) {
            const size_t meshRefL = hierarchyL->GetChildRef(k);
            this->AddSectorSuperStaticMesh(sector, ls.superStaticMeshes[meshRefL], ls);
        }
    } else {
        sector.sections.push_back({});
        LevelGeoSection& section = sector.sections.back();

        assert(!ssm->IsHierarchy() && !ssm->IsSkeleton());
        MyArray<MetroModelGeomData> gds;
        ssm->CollectGeomData(gds);
        assert(gds.size() == 1);

        const MetroModelGeomData& gd = gds.front();

        section.numVertices = gd.mesh->verticesCount;
        section.numIndices = gd.mesh->facesCount * 3;
        section.numShadowVertices = gd.mesh->shadowVerticesCount;
        section.numShadowIndices = gd.mesh->shadowFacesCount * 3;
        section.vbOffset = gd.mesh->verticesOffset * sizeof(VertexLevel);
        section.ibOffset = gd.mesh->indicesOffset * sizeof(uint16_t);
        section.shadowVBOffset = 0;
        section.shadowIBOffset = 0;
        section.surfaceIdx = mSurfaces.size();

        mSurfaces.push_back(ResourcesManager::Get().GetSurface(gd.texture));
        sector.bbox.Absorb(gd.bbox);
    }
}

bool LevelGeo::CreateTerrain() {
    static const size_t kTerrainChunkWidth = 128;
    static const size_t kTerrainChunkHeight = 128;

    mTerrainNumChunksX = mTerrainHMapWidth / kTerrainChunkWidth;
    mTerrainNumChunksY = mTerrainHMapHeight / kTerrainChunkHeight;

    assert(mTerrainNumChunksX * kTerrainChunkWidth == mTerrainHMapWidth);
    assert(mTerrainNumChunksY * kTerrainChunkHeight == mTerrainHMapHeight);

    const size_t numQuadsX = kTerrainChunkWidth - 1;
    const size_t numQuadsY = kTerrainChunkHeight - 1;
    const size_t numQuads = numQuadsX * numQuadsY;

    const size_t numIndicesPerChunk = numQuads * 6;

    MyArray<uint16_t> indices(numIndicesPerChunk);
    uint16_t* indicesPtr = indices.data();
    for (size_t y = 0; y < numQuadsY; ++y) {
        for (size_t x = 0; x < numQuadsX; ++x) {
            const size_t lt = x + (y * kTerrainChunkWidth);
            const size_t rt = lt + 1;
            const size_t lb = lt + kTerrainChunkWidth;
            const size_t rb = lb + 1;

            // triangle 1
            indicesPtr[0] = scast<uint16_t>(lt);
            indicesPtr[1] = scast<uint16_t>(lb);
            indicesPtr[2] = scast<uint16_t>(rt);
            // triangle 2
            indicesPtr[3] = scast<uint16_t>(rt);
            indicesPtr[4] = scast<uint16_t>(lb);
            indicesPtr[5] = scast<uint16_t>(rb);

            indicesPtr += 6;
        }
    }

    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA subData = {};
    HRESULT hr;

    ID3D11Device* device = Renderer::Get().GetDevice();

    //ib
    desc.ByteWidth = scast<UINT>(indices.size() * sizeof(uint16_t));
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subData.pSysMem = indices.data();
    subData.SysMemPitch = desc.ByteWidth;
    subData.SysMemSlicePitch = desc.ByteWidth;
    hr = device->CreateBuffer(&desc, &subData, &mTerrainIB);
    if (FAILED(hr)) {
        return false;
    }

    indices.clear();

    MyArray<vec2> instancesData; instancesData.reserve(mTerrainNumChunksX * mTerrainNumChunksY);
    mTerrainChunks.reserve(mTerrainNumChunksX * mTerrainNumChunksY);

    const vec3 dimChunk = vec3(mTerrainDim.x / scast<float>(mTerrainNumChunksX), mTerrainDim.y, mTerrainDim.z / scast<float>(mTerrainNumChunksY));
    for (size_t y = 0; y < mTerrainNumChunksY; ++y) {
        for (size_t x = 0; x < mTerrainNumChunksX; ++x) {
            const vec2 chunkPos = vec2(scast<float>(x * kTerrainChunkWidth), scast<float>(y * kTerrainChunkHeight));
            instancesData.push_back(chunkPos);

            TerrainChunk chunk;
            chunk.numIndices = numIndicesPerChunk;

            const vec3 minp = vec3(mTerrainMin.x + chunkPos.x, mTerrainMin.y, mTerrainMin.z + chunkPos.y);
            const vec3 maxp = vec3(minp.x + dimChunk.x, dimChunk.y, minp.z + dimChunk.z);

            chunk.bbox.minimum = MetroSwizzle(minp);
            chunk.bbox.maximum = MetroSwizzle(maxp);
            mTerrainChunks.push_back(chunk);
        }
    }

    //vb
    desc.ByteWidth = scast<UINT>(instancesData.size() * sizeof(vec2));
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    subData.pSysMem = instancesData.data();
    subData.SysMemPitch = desc.ByteWidth;
    subData.SysMemSlicePitch = desc.ByteWidth;
    hr = device->CreateBuffer(&desc, &subData, &mTerrainVB);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

} // namespace u4a
