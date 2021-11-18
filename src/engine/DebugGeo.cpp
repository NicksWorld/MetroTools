#include "DebugGeo.h"
#include <random>

#include "engine/Renderer.h"

#include <d3d11.h>

namespace u4a {

static std::mt19937 sRandomEngine;;


DebugGeo::DebugGeo()
    : mVertexBuffer(nullptr)
    , mIndexBuffer(nullptr)
{
}

DebugGeo::~DebugGeo() {
    this->Destroy();
}

void DebugGeo::AddSection(const vec3* vertices, const size_t numVertices, const uint16_t* indices, const size_t numIndices) {
    mInputSections.push_back(DebugInputSection{});
    DebugInputSection& inputSection = mInputSections.back();
    inputSection.indices.assign(indices, indices + numIndices);

    std::uniform_real_distribution<double> probability(0, 1);
    const double f0 = probability(sRandomEngine);
    const double f1 = probability(sRandomEngine);
    const double f2 = probability(sRandomEngine);

    const color32u c = Color4FTo32U(color4f(scast<float>(f0), scast<float>(f1), scast<float>(f2), 1.0f));

    inputSection.vertices.resize(numVertices);
    for (size_t i = 0; i < numVertices; ++i) {
        inputSection.vertices[i] = { vertices[i], c };
    }
}

bool DebugGeo::Create() {
    size_t totalVertices = 0;
    size_t totalIndices = 0;

    mSections.resize(mInputSections.size());
    for (size_t i = 0, numSections = mInputSections.size(); i < numSections; ++i) {
        DebugGeoSection& geoSection = mSections[i];
        geoSection.numVertices = scast<uint32_t>(mInputSections[i].vertices.size());
        geoSection.numIndices = scast<uint32_t>(mInputSections[i].indices.size());
        geoSection.vbOffset = scast<uint32_t>(totalVertices * sizeof(VertexDebug));
        geoSection.ibOffset = scast<uint32_t>(totalIndices * sizeof(uint16_t));

        totalVertices += geoSection.numVertices;
        totalIndices += geoSection.numIndices;
    }

    MyArray<VertexDebug> allVertices(totalVertices);
    MyArray<uint16_t> allIndices(totalIndices);
    VertexDebug* vbData = allVertices.data();
    uint16_t* ibData = allIndices.data();
    for (const DebugInputSection& inputSection : mInputSections) {
        memcpy(vbData, inputSection.vertices.data(), inputSection.vertices.size() * sizeof(VertexDebug));
        memcpy(ibData, inputSection.indices.data(), inputSection.indices.size() * sizeof(uint16_t));

        vbData += inputSection.vertices.size();
        ibData += inputSection.indices.size();
    }
    mInputSections.clear();

    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA subData = {};
    HRESULT hr;

    ID3D11Device* device = Renderer::Get().GetDevice();

    //vb
    desc.ByteWidth = scast<uint32_t>(totalVertices * sizeof(VertexDebug));
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    subData.pSysMem = allVertices.data();
    hr = device->CreateBuffer(&desc, &subData, &mVertexBuffer);
    if (FAILED(hr)) {
        return false;
    }

    //ib
    desc.ByteWidth = scast<uint32_t>(totalIndices * sizeof(uint16_t));
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subData.pSysMem = allIndices.data();
    hr = device->CreateBuffer(&desc, &subData, &mIndexBuffer);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void DebugGeo::Destroy() {
    MySafeRelease(mIndexBuffer);
    MySafeRelease(mIndexBuffer);
}

size_t DebugGeo::GetNumSections() const {
    return mSections.size();
}

const DebugGeoSection& DebugGeo::GetSection(const size_t idx) const {
    return mSections[idx];
}

ID3D11Buffer* DebugGeo::GetVertexBuffer() const {
    return mVertexBuffer;
}

ID3D11Buffer* DebugGeo::GetIndexBuffer() const {
    return mIndexBuffer;
}

} // namespace u4a
