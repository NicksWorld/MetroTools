#include "Model.h"

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"

#include "engine/Renderer.h"
#include "engine/ResourcesManager.h"

#include <d3d11.h>

namespace u4a {

Model::Model()
    : mType(Type::Static)
    , mVertexBuffer(nullptr)
    , mIndexBuffer(nullptr)
    , mSkeleton(nullptr)
{
}
Model::~Model() {
    this->Destroy();
}

const Model::Type Model::GetType() const {
    return mType;
}

const AABBox& Model::GetBBox() const {
    return mBBox;
}

const BSphere& Model::GetBSphere() const {
    return mBSphere;
}

const size_t Model::GetNumSections() const {
    return mSections.size();
}

const MeshSection& Model::GetSection(const size_t idx) const {
    return mSections[idx];
}

const Surface& Model::GetSurface(const size_t idx) const {
    return mSurfaces[idx];
}

const mat4 Model::GetBoneTransform(const CharString& boneName) const {
    if (mSkeleton) {
        const size_t idx = mSkeleton->FindBone(boneName);
        if (MetroBone::InvalidIdx != idx) {
            return mSkeleton->GetBoneFullTransform(idx);
        } else {
            return MatIdentity;
        }
    } else {
        return MatIdentity;
    }
}

bool Model::HasTransparency() const {
    return false;
}

bool Model::HasTranslucency() const {
    return false;
}

bool Model::Create(const MetroModel* mdl) {
    const size_t numMeshes = mdl->GetNumMeshes();
    mSections.reserve(numMeshes);

    const size_t vertexSize = mdl->IsAnimated() ? sizeof(VertexSkinned) : sizeof(VertexStatic);

    const MetroSkeleton* skel = mdl->GetSkeleton();
    if (skel) {
        mSkeleton = new MetroSkeleton();
        mSkeleton->Clone(skel);
    }

    mBBox.Reset();

    uint32_t totalVertices = 0;
    uint32_t totalIndices = 0;
    for (size_t i = 0; i < numMeshes; ++i) {
        const MetroMesh* mesh = mdl->GetMesh(i);

        // empty mesh ???
        if (!mesh->numVertices || mesh->faces.empty() || mesh->isCollision) {
            continue;
        }

        MeshSection rms;
        rms.numVertices = scast<uint32_t>(mesh->numVertices);
        rms.numIndices = scast<uint32_t>(mesh->faces.size() * 3);
        rms.vbOffset = scast<uint32_t>(totalVertices * vertexSize);
        rms.ibOffset = scast<uint32_t>(totalIndices * sizeof(uint16_t));

        rms.numShadowVertices = 0;
        rms.numShadowIndices = 0;
        rms.shadowVBOffset = 0;
        rms.shadowIBOffset = 0;

        rms.vscale = mesh->vscale;
        rms.alphaCut = 0.0f;
        if (mesh->materials[1].find("use_aref=1") != CharString::npos) {
            rms.alphaCut = 0.5f;
        }

        mSections.push_back(rms);

        totalVertices += rms.numVertices;
        totalIndices += rms.numIndices;

        mSurfaces.push_back(ResourcesManager::Get().GetSurface(mesh->materials.front()));

        mBBox.Absorb(mesh->bbox);
    }

    mBSphere.center = mBBox.Center();
    mBSphere.radius = Max3(mBBox.Extent());

    if (!totalVertices || !totalIndices) {
        return false;
    }

    BytesArray allVertices(vertexSize * totalVertices);
    MyArray<uint16_t> allIndices(totalIndices);

    size_t vbOffset = 0, ibOffset = 0;
    for (size_t i = 0; i < numMeshes; ++i) {
        const MetroMesh* mesh = mdl->GetMesh(i);

        const size_t numVertices = mesh->numVertices;
        const size_t numFaces = mesh->faces.size();

        // empty mesh ???
        if (!numVertices || !numFaces || mesh->isCollision) {
            continue;
        }

        memcpy(allVertices.data() + (vbOffset * vertexSize), mesh->rawVB.data(), numVertices * vertexSize);
        memcpy(allIndices.data() + ibOffset, mesh->faces.data(), numFaces * sizeof(MetroFace));

        vbOffset += numVertices;
        ibOffset += numFaces * 3;
    }


    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA subData = {};
    HRESULT hr;

    ID3D11Device* device = Renderer::Get().GetDevice();

    //vb
    desc.ByteWidth = scast<UINT>(vertexSize * totalVertices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    subData.pSysMem = allVertices.data();
    hr = device->CreateBuffer(&desc, &subData, &mVertexBuffer);
    if (FAILED(hr)) {
        return false;
    }

    //ib
    desc.ByteWidth = totalIndices * sizeof(uint16_t);
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subData.pSysMem = allIndices.data();
    hr = device->CreateBuffer(&desc, &subData, &mIndexBuffer);
    if (FAILED(hr)) {
        return false;
    }

    if (mdl->IsAnimated()) {
        mType = Type::Skinned;
    } else {
        mType = Type::Static;
    }

    return true;
}

bool Model::CreateNew(const MetroModelBase* mdl) {
    MyArray<MetroModelGeomData> gds;
    mdl->CollectGeomData(gds);

    if (gds.empty()) {
        return false;
    }

    const uint32_t vertexSize = sizeof(VertexStatic);

    mBBox.Reset();

    uint32_t totalVertices = 0;
    uint32_t totalIndices = 0;
    for (const MetroModelGeomData& gd : gds) {
        MeshSection rms;
        rms.numVertices = scast<uint32_t>(gd.mesh->verticesCount);
        rms.numIndices = scast<uint32_t>(gd.mesh->facesCount * 3);
        rms.vbOffset = scast<uint32_t>(totalVertices * vertexSize);
        rms.ibOffset = scast<uint32_t>(totalIndices * sizeof(uint16_t));

        rms.numShadowVertices = 0;
        rms.numShadowIndices = 0;
        rms.shadowVBOffset = 0;
        rms.shadowIBOffset = 0;

        rms.vscale = gd.mesh->verticesScale;
        rms.alphaCut = 0.0f;
        //if (mesh->materials[1].find("use_aref=1") != CharString::npos) {
        //    rms.alphaCut = 0.5f;
        //}

        mSections.push_back(rms);

        totalVertices += rms.numVertices;
        totalIndices += rms.numIndices;

        mSurfaces.push_back(ResourcesManager::Get().GetSurface(gd.texture));

        mBBox.Absorb(gd.bbox);
    }

    mBSphere.center = mBBox.Center();
    mBSphere.radius = Length(mBBox.Extent());

    BytesArray allVertices(vertexSize * totalVertices);
    MyArray<uint16_t> allIndices(totalIndices);

    size_t vbOffset = 0, ibOffset = 0;
    for (const MetroModelGeomData& gd : gds) {
        memcpy(allVertices.data() + (vbOffset * vertexSize), gd.vertices, gd.mesh->verticesCount * vertexSize);
        memcpy(allIndices.data() + ibOffset, gd.faces, gd.mesh->facesCount * sizeof(MetroFace));

        vbOffset += gd.mesh->verticesCount;
        ibOffset += gd.mesh->facesCount * 3;
    }

    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA subData = {};
    HRESULT hr;

    ID3D11Device* device = Renderer::Get().GetDevice();

    //vb
    desc.ByteWidth = scast<UINT>(vertexSize * totalVertices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    subData.pSysMem = allVertices.data();
    hr = device->CreateBuffer(&desc, &subData, &mVertexBuffer);
    if (FAILED(hr)) {
        return false;
    }

    //ib
    desc.ByteWidth = totalIndices * sizeof(uint16_t);
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subData.pSysMem = allIndices.data();
    hr = device->CreateBuffer(&desc, &subData, &mIndexBuffer);
    if (FAILED(hr)) {
        return false;
    }

    const uint32_t vtype = gds.front().mesh->vertexType;
    if (MetroVertexType::Skin == vtype) {
        mType = Type::Skinned;
    } else {
        mType = Type::Static;
    }

    return true;
}

void Model::Destroy() {
    MySafeRelease(mVertexBuffer);
    MySafeRelease(mIndexBuffer);

    mSections.clear();
    mSurfaces.clear();

    MySafeDelete(mSkeleton);
}

ID3D11Buffer* Model::GetVertexBuffer() const {
    return mVertexBuffer;
}

ID3D11Buffer* Model::GetIndexBuffer() const {
    return mIndexBuffer;
}

const MetroSkeleton* Model::GetSkeleton() const {
    return mSkeleton;
}

} // namespace u4a
