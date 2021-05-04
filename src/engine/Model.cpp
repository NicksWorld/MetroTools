#include "Model.h"

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"

#include "engine/Renderer.h"
#include "engine/ResourcesManager.h"

#include <d3d11.h>

namespace u4a {

Model::Model()
    : mType(Type::Static)
    , mBBox{}
    , mBSphere{}
    , mVertexBuffer(nullptr)
    , mIndexBuffer(nullptr)
    , mSkeleton{}
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

const size_t Model::GetNumSections(const size_t lodIdx) const {
    return mRenderLods[lodIdx].sections.size();
}

const MeshSection& Model::GetSection(const size_t idx, const size_t lodIdx) const {
    return mRenderLods[lodIdx].sections[idx];
}

const Surface& Model::GetSurface(const size_t idx, const size_t lodIdx) const {
    return mRenderLods[lodIdx].surfaces[idx];
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

bool Model::Create(const MetroModelBase* mdl) {
    const size_t vertexSize = sizeof(VertexStatic);

    mBBox.Reset();

    const size_t numLodsTotal = mdl->GetLodCount() + 1;
    mRenderLods.resize(numLodsTotal);

    size_t totalVertices = 0;
    size_t totalIndices = 0;

    uint32_t vtype = MetroVertexType::Static;

    for (size_t i = 0; i < numLodsTotal; ++i) {
        MyArray<MetroModelGeomData> gds;
        mdl->CollectGeomData(gds, (i == 0) ? kInvalidValue : (i - 1));

        if (!i && gds.empty()) {
            return false;
        }

        RenderLod& lod = mRenderLods[i];
        lod.sections.reserve(gds.size());
        lod.surfaces.reserve(gds.size());

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

            rms.bonesRemap = gd.mesh->bonesRemap;

            lod.sections.push_back(rms);

            totalVertices += rms.numVertices;
            totalIndices += rms.numIndices;

            lod.surfaces.push_back(ResourcesManager::Get().GetSurface(gd.texture));

            if (i == 0) {
                mBBox.Absorb(gd.bbox);
            }
        }

        if (i == 0) {
            vtype = gds.front().mesh->vertexType;
        }
    }

    mBSphere.center = mBBox.Center();
    mBSphere.radius = Max3(mBBox.Extent());

    BytesArray allVertices(vertexSize * totalVertices);
    MyArray<uint16_t> allIndices(totalIndices);

    size_t vbOffset = 0, ibOffset = 0;
    for (size_t i = 0; i < numLodsTotal; ++i) {
        MyArray<MetroModelGeomData> gds;
        mdl->CollectGeomData(gds, (i == 0) ? kInvalidValue : (i - 1));

        for (const MetroModelGeomData& gd : gds) {
            memcpy(allVertices.data() + vbOffset, gd.vertices, gd.mesh->verticesCount * vertexSize);
            memcpy(allIndices.data() + ibOffset, gd.faces, gd.mesh->facesCount * sizeof(MetroFace));

            vbOffset += gd.mesh->verticesCount * vertexSize;
            ibOffset += scast<size_t>(gd.mesh->facesCount) * 3;
        }
    }

    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA subData = {};
    HRESULT hr;

    ID3D11Device* device = Renderer::Get().GetDevice();

    //vb
    desc.ByteWidth = scast<UINT>(allVertices.size());
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

    if (MetroVertexType::Skin == vtype) {
        mType = Type::Skinned;
    } else if (MetroVertexType::Soft == vtype) {
        mType = Type::Soft;
    } else {
        mType = Type::Static;
    }

    if (mdl->IsSkeleton()) {
        mSkeleton = scast<const MetroModelSkeleton*>(mdl)->GetSkeleton();
    }

    return true;
}

void Model::Destroy() {
    MySafeRelease(mVertexBuffer);
    MySafeRelease(mIndexBuffer);

    mRenderLods.clear();

    mSkeleton = nullptr;
}

ID3D11Buffer* Model::GetVertexBuffer() const {
    return mVertexBuffer;
}

ID3D11Buffer* Model::GetIndexBuffer() const {
    return mIndexBuffer;
}

const RefPtr<MetroSkeleton>& Model::GetSkeleton() const {
    return mSkeleton;
}

} // namespace u4a
