#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include "Renderer.h"
#include "metro/MetroTypes.h"

#include "engine/Swapchain.h"
#if 1
#include "engine/Camera.h"
#include "engine/Model.h"
#include "engine/Animator.h"
#include "engine/LevelGeo.h"
#include "engine/DebugGeo.h"
#include "engine/Scene.h"
#include "engine/scenenodes/ModelNode.h"
#include "engine/scenenodes/LevelGeoNode.h"
#include "engine/scenenodes/DebugGeoNode.h"
#endif

#include "log.h"

#include "shaders/all_shaders.h"

namespace u4a {

vec4 Renderer::kClearColor[] = {
    vec4(40.0f / 255.0f, 113.0f / 255.0f, 134.0f / 255.0f, 1.0f),   // GBuffer Base
    vec4(0.5f, 0.5f, 0.5f, 1.0f),                                   // GBuffer Normals
};

const uint8_t   kStencilRefValue        = 42;
const size_t    kMaxDebugDrawVertices   = 4096;


Renderer::Renderer()
    : mDevice(nullptr)
    , mContext(nullptr)
    // Swapchain temp stuff
    , mBackBufferRTV(nullptr)
    , mBackBufferDSV(nullptr)
    // GBuffer
    , mGBufferWidth(0)
    , mGBufferHeight(0)
    // Selection target
    , mSelectionRT(nullptr)
    , mSelectionRTV(nullptr)
    , mSelectionReadback(nullptr)
    , mSelectionWidth(0)
    , mSelectionHeight(0)
    // Rasterizer states
    , mFillRS(nullptr)
    , mFillRSNoCull(nullptr)
    , mWireframeRS(nullptr)
    // Depth-stencil states
    , mDepthStencilState(nullptr)
    , mDepthStencilStateResolve(nullptr)
    , mDepthStencilStateDebug(nullptr)
    // Built-in shaders
    // vs
    , mVertexShaderStatic(nullptr)
    , mVertexShaderSkinned(nullptr)
    , mVertexShaderLevelGeo(nullptr)
    , mVertexShaderSoft(nullptr)
    , mVertexShaderTerrain(nullptr)
    , mVertexShaderFullscreen(nullptr)
    , mVertexShaderDebug(nullptr)
    , mVertexShaderDebugPassthrough(nullptr)
    // gs
    , mGeometryShaderDebugGenNormals(nullptr)
    // ps
    , mPixelShaderDefault(nullptr)
    , mPixelShaderTerrain(nullptr)
    , mPixelShaderDeferredResolve(nullptr)
    , mPixelShaderDeferredDebug(nullptr)
    , mPixelShaderDebug(nullptr)
    , mPixelShaderDebugLit(nullptr)
    , mPixelShaderSelection(nullptr)
    // Vertex input layouts
    , mInputLayoutStatic(nullptr)
    , mInputLayoutSkinned(nullptr)
    , mInputLayoutLevelGeo(nullptr)
    , mInputLayoutSoft(nullptr)
    , mInputLayoutTerrain(nullptr)
    , mInputLayoutDebug(nullptr)
    // Samplers states
    , mPointSampler(nullptr)
    , mTrilinearSampler(nullptr)
    , mAnisotropicSampler(nullptr)
    // Constant buffers
    , mCBMatrices(nullptr)
    , mCBSkinned(nullptr)
    , mCBTerrain(nullptr)
    , mCBSurfParams(nullptr)
    //
    , mUpdateFrustum(true)
    , mRendererType(RendererType::Regular)
    // debug
    , mDebugVertexBuffer(nullptr)
    , mDebugVerticesPtr(nullptr)
    , mDebugVerticesCount(0)
{
    memset(mGBufferRT, 0, sizeof(mGBufferRT));
    memset(mGBufferRTV, 0, sizeof(mGBufferRTV));
    memset(mGBufferSRV, 0, sizeof(mGBufferSRV));

    mUpdateFrustum = true;
}
Renderer::~Renderer() {

}

bool Renderer::CreateDevice(const size_t flags) {
    const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT deviceFlags = 0;
#ifdef _DEBUG
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    if (TestBit<size_t>(flags, InitFlags::IF_D2D_Support)) {
        deviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    }

    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
                                   nullptr, deviceFlags, &featureLevel, 1,
                                   D3D11_SDK_VERSION, &mDevice, nullptr, &mContext);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create device, hr = %d", hr);
        return false;
    }

#ifdef _DEBUG
    ID3D11InfoQueue* dbgInfoQueue = nullptr;
    if (SUCCEEDED(mDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&dbgInfoQueue)) && dbgInfoQueue) {
        //#NOTE_SK: D3D12 Errors are very important, so we tell debug layer to break on any error right away
        //          so we can see where and why it happened and fix it
        dbgInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        dbgInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        dbgInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif


    return true;
}

bool Renderer::Initialize() {
    HRESULT hr;

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.ScissorEnable = FALSE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;

    hr = mDevice->CreateRasterizerState(&rasterDesc, &mFillRS);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create fill rs, hr = %d", hr);
        return false;
    }

    rasterDesc.CullMode = D3D11_CULL_NONE;
    hr = mDevice->CreateRasterizerState(&rasterDesc, &mFillRSNoCull);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create fill (no cull) rs, hr = %d", hr);
        return false;
    }

    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    hr = mDevice->CreateRasterizerState(&rasterDesc, &mWireframeRS);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create wireframe rs, hr = %d", hr);
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = TRUE;
    depthStencilDesc.StencilReadMask = 0;
    depthStencilDesc.StencilWriteMask = 0xFF;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
    hr = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create DS fill, hr = %d", hr);
        return false;
    }

    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.StencilEnable = TRUE;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
    hr = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilStateResolve);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create DS resolve, hr = %d", hr);
        return false;
    }

    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;//D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;
    depthStencilDesc.StencilReadMask = 0;
    depthStencilDesc.StencilWriteMask = 0;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
    hr = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilStateDebug);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create DS debug, hr = %d", hr);
        return false;
    }

    // create built-in shaders
    if (!this->CreateShaders()) {
        return false;
    }

    // create vertex input layouts
    if (!this->CreateVertexInputLayouts()) {
        return false;
    }

    // create samplers
    if (!this->CreateSamplers()) {
        return false;
    }

    // create constant buffers
    if (!this->CreateConstantBuffers()) {
        return false;
    }

    // debug
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = scast<UINT>(sizeof(VertexDebug) * kMaxDebugDrawVertices);
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = mDevice->CreateBuffer(&vbDesc, nullptr, &mDebugVertexBuffer);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create debug VB, hr = %d", hr);
        return false;
    }

    return true;
}


void Renderer::Shutdown() {
}

ID3D11Device* Renderer::GetDevice() {
    return mDevice;
}

ID3D11DeviceContext* Renderer::GetContext() {
    return mContext;
}

void Renderer::StartFrame(Swapchain& swapchain) {
    this->CheckGBuffer(swapchain);

    mBackBufferRTV = swapchain.GetRTV();
    mBackBufferDSV = swapchain.GetDSV();

    D3D11_VIEWPORT viewport = {};
    viewport.Width = scast<float>(swapchain.GetWidth());
    viewport.Height = scast<float>(swapchain.GetHeight());
    viewport.MaxDepth = 1.0f;
    mContext->RSSetViewports(1, &viewport);
}

void Renderer::DrawScene(Scene& scene, const size_t flags) {
#if 1
    Camera* camera = scene.GetCamera();
    if (camera) {
        mCBMatricesData.View = camera->GetTransform();
        mCBMatricesData.Projection = camera->GetProjection();

        if (mUpdateFrustum) {
            mat4 viewProj = mCBMatricesData.Projection * mCBMatricesData.View;
            mFrustum.FromMatrix(viewProj);
        }
    }

    ID3D11SamplerState* samplers[] = { mPointSampler, mTrilinearSampler, mAnisotropicSampler };
    mContext->PSSetSamplers(0, 3, samplers);
    mContext->VSSetSamplers(0, 1, samplers);

    mModelNodes.resize(0);
    if (!TestBit<size_t>(flags, Renderer::DF_SkipModels)) {
        scene.CollectNodesByType(SceneNode::NodeType::Model, mModelNodes);
    }
    mLevelGeoNodes.resize(0);
    if (!TestBit<size_t>(flags, Renderer::DF_SkipLevelGeo)) {
        scene.CollectNodesByType(SceneNode::NodeType::LevelGeo, mLevelGeoNodes);
    }
    mDebugGeoNodes.resize(0);
    if (!TestBit<size_t>(flags, Renderer::DF_SkipDebugGeo)) {
        scene.CollectNodesByType(SceneNode::NodeType::DebugGeo, mDebugGeoNodes);
    }

    // GBuffer fill pass
    {
        if (mRendererType != RendererType::Wireframe) {
            mContext->OMSetRenderTargets(scast<UINT>(GBuffer_NumLayers), mGBufferRTV, mBackBufferDSV);
            for (size_t i = 0; i < GBuffer_NumLayers; ++i) {
                mContext->ClearRenderTargetView(mGBufferRTV[i], &kClearColor[i].x);
            }
            mContext->OMSetDepthStencilState(mDepthStencilState, kStencilRefValue);
            mContext->ClearDepthStencilView(mBackBufferDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            mContext->RSSetState(mFillRS);
        } else {
            mContext->OMSetRenderTargets(1, &mBackBufferRTV, mBackBufferDSV);
            mContext->ClearRenderTargetView(mBackBufferRTV, &kClearColor[0].x);

            mContext->RSSetState(mWireframeRS);
        }

        this->DrawFillPass();
    }

    // Shading resolve pass
    if (mRendererType != RendererType::Wireframe) {
        mContext->OMSetRenderTargets(1, &mBackBufferRTV, mBackBufferDSV);
        mContext->ClearRenderTargetView(mBackBufferRTV, &kClearColor[0].x);

        mContext->RSSetState(mFillRS);
        mContext->OMSetDepthStencilState(mDepthStencilStateResolve, kStencilRefValue);

        this->DrawResolvePass();
    }

    this->DrawTools();

    if (!mDebugGeoNodes.empty()) {
        this->DrawDebug();
    }

#endif
}


vec4 RGBAUintToFloat(const uint32_t c) {
    vec4 rgba;
    rgba.x = (c         & 0xff) * 0.003921568627f;  //  /255.0f;
    rgba.y = ((c >> 8)  & 0xff) * 0.003921568627f;  //  /255.0f;
    rgba.z = ((c >> 16) & 0xff) * 0.003921568627f;  //  /255.0f;
    rgba.w = ((c >> 24) & 0xff) * 0.003921568627f;  //  /255.0f;
    return rgba;
}

SceneNode* Renderer::PickObject(Swapchain& swapchain, Scene& scene, const vec2& screenPoint) {
    this->CheckSelectionRT(swapchain);

    Camera* camera = scene.GetCamera();
    if (camera) {
        mCBMatricesData.View = camera->GetTransform();
        mCBMatricesData.Projection = camera->GetProjection();

        mat4 viewProj = mCBMatricesData.Projection * mCBMatricesData.View;
        mFrustum.FromMatrix(viewProj);
    }

    mModelNodes.resize(0);
    scene.CollectNodesByType(SceneNode::NodeType::Model, mModelNodes);

    Ray ray = camera->CalcRayFromScreenPoint(screenPoint);

    SceneNodesArray nodesToPick;
    for (SceneNode* n : mModelNodes) {
        ModelNode* node = scast<ModelNode*>(n);
        const AABBox& aabb = node->GetAABB();
        if (mFrustum.IsAABBoxIn(aabb) && RayAABBIntersection(ray, aabb)) {
            nodesToPick.push_back(node);
        }
    }

    SceneNode* result = nullptr;
    if (!nodesToPick.empty()) {
        const uint32_t kPickColourThreshold = 128u;
        const float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        uint32_t pickColour = kPickColourThreshold;

        mContext->OMSetRenderTargets(1, &mSelectionRTV, mBackBufferDSV);
        mContext->ClearRenderTargetView(mSelectionRTV, clearColour);
        mContext->OMSetDepthStencilState(mDepthStencilState, kStencilRefValue);
        mContext->ClearDepthStencilView(mBackBufferDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        mContext->RSSetState(mFillRS);

        mContext->VSSetConstantBuffers(0, 1, &mCBMatrices);
        mContext->PSSetConstantBuffers(0, 1, &mCBSurfParams);

        mContext->PSSetShader(mPixelShaderSelection, nullptr, 0);
        mContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for (SceneNode* n : nodesToPick) {
            ModelNode* node = scast<ModelNode*>(n);

            Model* model = node->GetModel();

            mCBMatricesData.Model = node->GetTransform();
            mCBMatricesData.ModelView = mCBMatricesData.View * mCBMatricesData.Model;
            mCBMatricesData.ModelViewProj = mCBMatricesData.Projection * mCBMatricesData.ModelView;
            mCBMatricesData.NormalWS = MatTranspose(MatInverse(mCBMatricesData.Model));

            D3D11_MAPPED_SUBRESOURCE mapped;
            mContext->Map(mCBMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            memcpy(mapped.pData, &mCBMatricesData, sizeof(mCBMatricesData));
            mContext->Unmap(mCBMatrices, 0);

            bool isDynamic = false;
            UINT stride;
            if (model->GetType() == Model::Type::Static) {
                mContext->IASetInputLayout(mInputLayoutStatic);
                mContext->VSSetShader(mVertexShaderStatic, nullptr, 0);
                stride = sizeof(VertexStatic);
            } else if (model->GetType() == Model::Type::Skinned) {
                mContext->IASetInputLayout(mInputLayoutSkinned);
                mContext->VSSetShader(mVertexShaderSkinned, nullptr, 0);
                stride = sizeof(VertexSkinned);
                isDynamic = true;
            } else if (model->GetType() == Model::Type::Soft) {
                mContext->IASetInputLayout(mInputLayoutSoft);
                mContext->VSSetShader(mVertexShaderSoft, nullptr, 0);
                stride = sizeof(VertexSoft);
            } else {
                assert(false && "Invalid model type!");
            }

            mCBSurfParamsData.param0 = RGBAUintToFloat(pickColour);

            mContext->Map(mCBSurfParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            memcpy(mapped.pData, &mCBSurfParamsData, sizeof(mCBSurfParamsData));
            mContext->Unmap(mCBSurfParams, 0);

            ID3D11Buffer* vb = model->GetVertexBuffer();
            ID3D11Buffer* ib = model->GetIndexBuffer();

            UINT vbOffset = 0, ibOffset = 0;
            const size_t numSections = model->GetNumSections();
            for (size_t i = 0; i < numSections; ++i) {
                const MeshSection& section = model->GetSection(i);
                const Surface& surface = model->GetSurface(i);

                if (isDynamic) {
                    mCBSkinnedData.VScale.x = section.vscale;

                    const Animator* anim = node->GetAnimator();
                    if (anim) {
                        //#TODO_SK: optimization - move vscale somewhere and memcpy matrices only once !!
                        const Mat4Array& animResult = anim->GetAnimResult();
                        memcpy(mCBSkinnedData.Bones, animResult.data(), animResult.size() * sizeof(mat4));
                    }

                    mContext->Map(mCBSkinned, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                    memcpy(mapped.pData, &mCBSkinnedData, sizeof(mCBSkinnedData));
                    mContext->Unmap(mCBSkinned, 0);
                }

                mContext->IASetVertexBuffers(0, 1, &vb, &stride, &vbOffset);
                mContext->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, ibOffset);
                mContext->DrawIndexed(scast<UINT>(section.numIndices), 0, 0);

                vbOffset += scast<UINT>(section.numVertices) * stride;
                ibOffset += scast<UINT>(section.numIndices) * sizeof(uint16_t);
            }

            pickColour += kPickColourThreshold;
        }

        D3D11_BOX copyBox = {};
        copyBox.left = scast<UINT>(screenPoint.x);
        copyBox.top = scast<UINT>(screenPoint.y);
        copyBox.right = copyBox.left + 1;
        copyBox.bottom = copyBox.top + 1;
        copyBox.back = 1;
        mContext->CopySubresourceRegion(mSelectionReadback, 0, 0, 0, 0, mSelectionRT, 0, &copyBox);

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(mContext->Map(mSelectionReadback, 0, D3D11_MAP_READ, 0, &mapped))) {
            uint32_t pixelColour = *rcast<uint32_t*>(mapped.pData);
            mContext->Unmap(mSelectionReadback, 0);

            if (pixelColour >= kPickColourThreshold) {
                const size_t idx = (pixelColour - kPickColourThreshold) / kPickColourThreshold;
                if (idx < nodesToPick.size()) {
                    result = nodesToPick[idx];
                }
            }
        }
    }

    return result;
}

void Renderer::SetRendererType(const RendererType type) {
    mRendererType = type;
}

void Renderer::BeginDebugDraw() {
}

void Renderer::DebugDrawLine(const vec3& pt0, const vec3& pt1, const color4f& color) {
    this->EnsureDebugVertices(2);

    const color32u c = Color4FTo32U(color);

    VertexDebug* vbStart = rcast<VertexDebug*>(mDebugVerticesPtr) + mDebugVerticesCount;
    vbStart[0] = { pt0, c };
    vbStart[1] = { pt1, c };

    mDebugVerticesCount += 2;
}

void Renderer::DebugDrawBBox(const AABBox& bbox, const color4f& color) {
    constexpr size_t kNumBoxVertices = 24;

    this->EnsureDebugVertices(kNumBoxVertices);

    const color32u c = Color4FTo32U(color);

    vec3 points[8] = {
        bbox.minimum,
        vec3(bbox.maximum.x, bbox.minimum.y, bbox.minimum.z),
        vec3(bbox.maximum.x, bbox.minimum.y, bbox.maximum.z),
        vec3(bbox.minimum.x, bbox.minimum.y, bbox.maximum.z),
        vec3(bbox.minimum.x, bbox.maximum.y, bbox.minimum.z),
        vec3(bbox.maximum.x, bbox.maximum.y, bbox.minimum.z),
        bbox.maximum,
        vec3(bbox.minimum.x, bbox.maximum.y, bbox.maximum.z),
    };

    VertexDebug* vbStart = rcast<VertexDebug*>(mDebugVerticesPtr) + mDebugVerticesCount;
    vbStart[ 0] = { points[0], c };  vbStart[ 1] = { points[1], c };
    vbStart[ 2] = { points[1], c };  vbStart[ 3] = { points[2], c };
    vbStart[ 4] = { points[2], c };  vbStart[ 5] = { points[3], c };
    vbStart[ 6] = { points[3], c };  vbStart[ 7] = { points[0], c };
    vbStart[ 8] = { points[4], c };  vbStart[ 9] = { points[5], c };
    vbStart[10] = { points[5], c };  vbStart[11] = { points[6], c };
    vbStart[12] = { points[6], c };  vbStart[13] = { points[7], c };
    vbStart[14] = { points[7], c };  vbStart[15] = { points[4], c };
    vbStart[16] = { points[0], c };  vbStart[17] = { points[4], c };
    vbStart[18] = { points[1], c };  vbStart[19] = { points[5], c };
    vbStart[20] = { points[2], c };  vbStart[21] = { points[6], c };
    vbStart[22] = { points[3], c };  vbStart[23] = { points[7], c };

    mDebugVerticesCount += kNumBoxVertices;
}

void Renderer::DebugDrawRing(const vec3& origin, const vec3& majorAxis, const vec3& minorAxis, const color4f& color) {
    const color32u c = Color4FTo32U(color);

    constexpr size_t kNumRingSegments = 32;
    constexpr size_t kNumRingVertices = kNumRingSegments * 2;

    this->EnsureDebugVertices(kNumRingVertices);

    VertexDebug* vbStart = rcast<VertexDebug*>(mDebugVerticesPtr) + mDebugVerticesCount;
    size_t vbOffset = 0;

    constexpr float angleDelta = MM_TwoPi / scast<float>(kNumRingSegments);
    // Instead of calling cos/sin for each segment we calculate
    // the sign of the angle delta and then incrementally calculate sin
    // and cosine from then on.
    float cosDelta = std::cosf(angleDelta);
    float sinDelta = std::sinf(angleDelta);
    float incrementalSin = 0.0f;
    float incrementalCos = 1.0f;

    vec3 firstPoint = majorAxis + origin;
    vec3 prevPoint = firstPoint;

    for (size_t i = 1; i < kNumRingSegments; ++i) {
        const float newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
        const float newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
        incrementalCos = newCos;
        incrementalSin = newSin;

        vec3 point = (majorAxis * incrementalCos + origin) + (minorAxis * incrementalSin);

        vbStart[vbOffset++] = { prevPoint, c };
        vbStart[vbOffset++] = { point, c };

        prevPoint = point;
    }

    vbStart[vbOffset++] = { prevPoint, c };
    vbStart[vbOffset++] = { firstPoint, c };

    mDebugVerticesCount += kNumRingVertices;
}

void Renderer::DebugDrawBSphere(const BSphere& bsphere, const color4f& color) {
    const vec3 xaxis = vec3(bsphere.radius, 0.0f, 0.0f);
    const vec3 yaxis = vec3(0.0f, bsphere.radius, 0.0f);
    const vec3 zaxis = vec3(0.0f, 0.0f, bsphere.radius);

    this->DebugDrawRing(bsphere.center, xaxis, zaxis, color);
    this->DebugDrawRing(bsphere.center, xaxis, yaxis, color);
    this->DebugDrawRing(bsphere.center, yaxis, zaxis, color);
}

void Renderer::DebugDrawTetrahedron(const vec3& a, const vec3& b, const float r, const color4f& color) {
    constexpr size_t kNumTetrahedronLines = 6;
    constexpr size_t kNumTetrahedronVertices = kNumTetrahedronLines * 2;

    this->EnsureDebugVertices(kNumTetrahedronVertices);

    const color32u c = Color4FTo32U(color);

    vec3 dir = Normalize(b - a);
    vec3 axisX, axisZ;
    OrthonormalBasis(dir, axisX, axisZ);

    const vec3 p0 = a - vec3(axisX * r + axisZ * r);
    const vec3 p1 = a + vec3(axisX * r + (-axisZ * r));
    const vec3 p2 = a + vec3(axisZ * r);

    VertexDebug* vbStart = rcast<VertexDebug*>(mDebugVerticesPtr) + mDebugVerticesCount;

    // base
    vbStart[ 0] = { p0, c }; vbStart[ 1] = { p1, c };
    vbStart[ 2] = { p1, c }; vbStart[ 3] = { p2, c };
    vbStart[ 4] = { p2, c }; vbStart[ 5] = { p0, c };
    // top
    vbStart[ 6] = { p0, c }; vbStart[ 7] = { b, c };
    vbStart[ 8] = { p1, c }; vbStart[ 9] = { b, c };
    vbStart[10] = { p2, c }; vbStart[11] = { b, c };

    mDebugVerticesCount += kNumTetrahedronVertices;
}

void Renderer::EndDebugDraw() {
    this->FlushDebugVertices();
}



void Renderer::EnsureDebugVertices(const size_t required) {
    if (mDebugVerticesCount + required >= kMaxDebugDrawVertices || !mDebugVerticesPtr) {
        this->FlushDebugVertices();

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = mContext->Map(mDebugVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            mDebugVerticesPtr = mapped.pData;
        }

        mDebugVerticesCount = 0;
    }
}

void Renderer::FlushDebugVertices() {
    if (mDebugVerticesPtr) {
        mContext->Unmap(mDebugVertexBuffer, 0);
        mDebugVerticesPtr = nullptr;
    }

    if (mDebugVerticesCount) {
        mContext->OMSetRenderTargets(1, &mBackBufferRTV, mBackBufferDSV);
        mContext->RSSetState(mFillRSNoCull);
        mContext->OMSetDepthStencilState(mDepthStencilStateDebug, 0);

        mContext->VSSetShader(mVertexShaderDebug, nullptr, 0);
        mContext->PSSetShader(mPixelShaderDebug, nullptr, 0);

        mContext->IASetInputLayout(mInputLayoutDebug);
        const UINT stride = sizeof(VertexDebug);
        const UINT offset = 0;
        mContext->IASetVertexBuffers(0, 1, &mDebugVertexBuffer, &stride, &offset);

        mCBMatricesData.ModelViewProj = mCBMatricesData.Projection * mCBMatricesData.View;

        D3D11_MAPPED_SUBRESOURCE mapped;
        mContext->Map(mCBMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &mCBMatricesData, sizeof(mCBMatricesData));
        mContext->Unmap(mCBMatrices, 0);

        mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

        mContext->Draw(scast<UINT>(mDebugVerticesCount), 0);
    }

    mDebugVerticesCount = 0;
}

void Renderer::CheckGBuffer(const Swapchain& swapchain) {
    const size_t w = swapchain.GetWidth();
    const size_t h = swapchain.GetHeight();

    if (w != mGBufferWidth || h != mGBufferHeight || !mGBufferRT[0]) {
        const DXGI_FORMAT resFormats[GBuffer_NumLayers] = {
            DXGI_FORMAT_R8G8B8A8_TYPELESS,
            DXGI_FORMAT_R11G11B10_FLOAT
        };
        const DXGI_FORMAT viewFormats[GBuffer_NumLayers] = {
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            DXGI_FORMAT_R11G11B10_FLOAT
        };

        for (size_t i = 0; i < GBuffer_NumLayers; ++i) {
            MySafeRelease(mGBufferRT[i]);
            MySafeRelease(mGBufferRTV[i]);
            MySafeRelease(mGBufferSRV[i]);

            D3D11_TEXTURE2D_DESC resourceDesc = {};
            resourceDesc.Width = scast<UINT>(w);
            resourceDesc.Height = scast<UINT>(h);
            resourceDesc.MipLevels = 1;
            resourceDesc.ArraySize = 1;
            resourceDesc.Format = resFormats[i];
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Usage = D3D11_USAGE_DEFAULT;
            resourceDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

            HRESULT hr = mDevice->CreateTexture2D(&resourceDesc, nullptr, &mGBufferRT[i]);
            if (SUCCEEDED(hr)) {
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = viewFormats[i];
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                mDevice->CreateRenderTargetView(mGBufferRT[i], &rtvDesc, &mGBufferRTV[i]);

                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = viewFormats[i];
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;
                mDevice->CreateShaderResourceView(mGBufferRT[i], &srvDesc, &mGBufferSRV[i]);
            }
        }

        mGBufferWidth = w;
        mGBufferHeight = h;
    }
}

void Renderer::CheckSelectionRT(const Swapchain& swapchain) {
    const size_t w = swapchain.GetWidth();
    const size_t h = swapchain.GetHeight();

    if (w != mSelectionWidth || h != mSelectionHeight || !mSelectionRT) {
        const DXGI_FORMAT resFormats[GBuffer_NumLayers] = {
            DXGI_FORMAT_R8G8B8A8_TYPELESS,
            DXGI_FORMAT_R11G11B10_FLOAT
        };
        const DXGI_FORMAT viewFormats[GBuffer_NumLayers] = {
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            DXGI_FORMAT_R11G11B10_FLOAT
        };

        MySafeRelease(mSelectionRT);
        MySafeRelease(mSelectionRTV);

        D3D11_TEXTURE2D_DESC resourceDesc = {};
        resourceDesc.Width = scast<UINT>(w);
        resourceDesc.Height = scast<UINT>(h);
        resourceDesc.MipLevels = 1;
        resourceDesc.ArraySize = 1;
        resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Usage = D3D11_USAGE_DEFAULT;
        resourceDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

        HRESULT hr = mDevice->CreateTexture2D(&resourceDesc, nullptr, &mSelectionRT);
        if (SUCCEEDED(hr)) {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            mDevice->CreateRenderTargetView(mSelectionRT, &rtvDesc, &mSelectionRTV);
        }

        mSelectionWidth = w;
        mSelectionHeight = h;
    }

    if (!mSelectionReadback) {
        D3D11_TEXTURE2D_DESC resourceDesc = {};
        resourceDesc.Width = 1;
        resourceDesc.Height = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.ArraySize = 1;
        resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Usage = D3D11_USAGE_STAGING;
        resourceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        mDevice->CreateTexture2D(&resourceDesc, nullptr, &mSelectionReadback);
    }
}


bool Renderer::CreateShaders() {
    HRESULT hr;

    ///// Vertex shaders

    // static vertex
    hr = mDevice->CreateVertexShader(sVS_StaticDataPtr, sVS_StaticDataLen, nullptr, &mVertexShaderStatic);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create static VS, hr = %d", hr);
        return false;
    }

    // skinned vertex
    hr = mDevice->CreateVertexShader(sVS_SkinnedDataPtr, sVS_SkinnedDataLen, nullptr, &mVertexShaderSkinned);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create skinned VS, hr = %d", hr);
        return false;
    }

    // level vertex
    hr = mDevice->CreateVertexShader(sVS_LevelDataPtr, sVS_LevelDataLen, nullptr, &mVertexShaderLevelGeo);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create level VS, hr = %d", hr);
        return false;
    }

    // soft vertex
    hr = mDevice->CreateVertexShader(sVS_SoftDataPtr, sVS_SoftDataLen, nullptr, &mVertexShaderSoft);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create soft VS, hr = %d", hr);
        return false;
    }

    // level terrain
    hr = mDevice->CreateVertexShader(sVS_TerrainDataPtr, sVS_TerrainDataLen, nullptr, &mVertexShaderTerrain);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create terrain VS, hr = %d", hr);
        return false;
    }

    // fullscreen triangle
    hr = mDevice->CreateVertexShader(sVS_FullscreenTriangleDataPtr, sVS_FullscreenTriangleDataLen, nullptr, &mVertexShaderFullscreen);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create fullscreen VS, hr = %d", hr);
        return false;
    }

    // debug drawing
    hr = mDevice->CreateVertexShader(sVS_DebugDataPtr, sVS_DebugDataLen, nullptr, &mVertexShaderDebug);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create debug VS, hr = %d", hr);
        return false;
    }

    // debug drawing (passthrough)
    hr = mDevice->CreateVertexShader(sVS_DebugPassthroughDataPtr, sVS_DebugPassthroughDataLen, nullptr, &mVertexShaderDebugPassthrough);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create debug (passthrough) VS, hr = %d", hr);
        return false;
    }

    ///// Geometry shaders

    // generate debug geo normals
    hr = mDevice->CreateGeometryShader(sGS_DebugGenNormalsDataPtr, sGS_DebugGenNormalsDataLen, nullptr, &mGeometryShaderDebugGenNormals);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create debug GS, hr = %d", hr);
        return false;
    }

    ///// Pixel shaders

    // default material
    hr = mDevice->CreatePixelShader(sPS_DefaultDataPtr, sPS_DefaultDataLen, nullptr, &mPixelShaderDefault);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create default fill PS, hr = %d", hr);
        return false;
    }

    // terrain material
    hr = mDevice->CreatePixelShader(sPS_TerrainDataPtr, sPS_TerrainDataLen, nullptr, &mPixelShaderTerrain);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create terrain fill PS, hr = %d", hr);
        return false;
    }

    // deferred resolve
    hr = mDevice->CreatePixelShader(sPS_DeferredResolveDataPtr, sPS_DeferredResolveDataLen, nullptr, &mPixelShaderDeferredResolve);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create deferred resolve PS, hr = %d", hr);
        return false;
    }

    // deferred debug
    hr = mDevice->CreatePixelShader(sPS_DeferredDebugDataPtr, sPS_DeferredDebugDataLen, nullptr, &mPixelShaderDeferredDebug);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create deferred debug PS, hr = %d", hr);
        return false;
    }

    // debug drawing
    hr = mDevice->CreatePixelShader(sPS_DebugDataPtr, sPS_DebugDataLen, nullptr, &mPixelShaderDebug);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create debug PS, hr = %d", hr);
        return false;
    }

    // debug lit drawing
    hr = mDevice->CreatePixelShader(sPS_DebugLitDataPtr, sPS_DebugLitDataLen, nullptr, &mPixelShaderDebugLit);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create debug lit PS, hr = %d", hr);
        return false;
    }

    // selection drawing
    hr = mDevice->CreatePixelShader(sPS_SelectionDataPtr, sPS_SelectionDataLen, nullptr, &mPixelShaderSelection);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create selection PS, hr = %d", hr);
        return false;
    }

    return true;
}

bool Renderer::CreateVertexInputLayouts() {
    HRESULT hr;

    // static vertex
    D3D11_INPUT_ELEMENT_DESC iedStatic[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexStatic, pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, offsetof(VertexStatic, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",   0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, offsetof(VertexStatic, aux0),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, offsetof(VertexStatic, aux1),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(VertexStatic, uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // skinned vertex
    D3D11_INPUT_ELEMENT_DESC iedSkinned[] = {
        { "POSITION",  0, DXGI_FORMAT_R16G16B16A16_UINT, 0, offsetof(VertexSkinned, pos),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexSkinned, normal),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",   0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexSkinned, aux0),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexSkinned, aux1),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONES",     0, DXGI_FORMAT_R8G8B8A8_UINT,     0, offsetof(VertexSkinned, bones),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "WEIGHTS",   0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexSkinned, weights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R16G16_SINT,       0, offsetof(VertexSkinned, uv),      D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // level-geo vertex
    D3D11_INPUT_ELEMENT_DESC iedLevelGeo[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,   0, offsetof(VertexLevel, pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexLevel, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",   0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexLevel, aux0),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexLevel, aux1),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R16G16B16A16_SINT, 0, offsetof(VertexLevel, uv0),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // soft vertex
    D3D11_INPUT_ELEMENT_DESC iedSoft[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,   0, offsetof(VertexSoft, pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",   0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexSoft, aux0),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,   0, offsetof(VertexSoft, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R16G16_SINT,       0, offsetof(VertexSoft, uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // level-geo vertex
    D3D11_INPUT_ELEMENT_DESC iedTerrain[] = {
        { "INSTDATA",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };

    // debug vertex
    D3D11_INPUT_ELEMENT_DESC iedDebug[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,   0, offsetof(VertexDebug, pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, offsetof(VertexDebug, color),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // static vertex
    hr = mDevice->CreateInputLayout(iedStatic,
                                    scast<UINT>(std::size(iedStatic)),
                                    sVS_StaticDataPtr,
                                    sVS_StaticDataLen,
                                    &mInputLayoutStatic);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create static IL, hr = %d", hr);
        return false;
    }

    // skinned vertex
    hr = mDevice->CreateInputLayout(iedSkinned,
                                    scast<UINT>(std::size(iedSkinned)),
                                    sVS_SkinnedDataPtr,
                                    sVS_SkinnedDataLen,
                                    &mInputLayoutSkinned);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create skinned IL, hr = %d", hr);
        return false;
    }

    // level vertex
    hr = mDevice->CreateInputLayout(iedLevelGeo,
                                    scast<UINT>(std::size(iedLevelGeo)),
                                    sVS_LevelDataPtr,
                                    sVS_LevelDataLen,
                                    &mInputLayoutLevelGeo);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create level IL, hr = %d", hr);
        return false;
    }

    // soft vertex
    hr = mDevice->CreateInputLayout(iedSoft,
                                    scast<UINT>(std::size(iedSoft)),
                                    sVS_SoftDataPtr,
                                    sVS_SoftDataLen,
                                    &mInputLayoutSoft);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create soft IL, hr = %d", hr);
        return false;
    }

    // terrain vertex
    hr = mDevice->CreateInputLayout(iedTerrain,
                                    scast<UINT>(std::size(iedTerrain)),
                                    sVS_TerrainDataPtr,
                                    sVS_TerrainDataLen,
                                    &mInputLayoutTerrain);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create terrain IL, hr = %d", hr);
        return false;
    }

    // debug vertex
    hr = mDevice->CreateInputLayout(iedDebug,
                                    scast<UINT>(std::size(iedDebug)),
                                    sVS_DebugDataPtr,
                                    sVS_DebugDataLen,
                                    &mInputLayoutDebug);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create debug IL, hr = %d", hr);
        return false;
    }

    return true;
}

bool Renderer::CreateSamplers() {
    HRESULT hr;

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = -(std::numeric_limits<float>::max)();
    samplerDesc.MaxLOD = (std::numeric_limits<float>::max)();

    hr = mDevice->CreateSamplerState(&samplerDesc, &mPointSampler);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create point sampler, hr = %d", hr);
        return false;
    }

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    hr = mDevice->CreateSamplerState(&samplerDesc, &mTrilinearSampler);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create trilinear sampler, hr = %d", hr);
        return false;
    }

    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.MaxAnisotropy = 4;

    hr = mDevice->CreateSamplerState(&samplerDesc, &mAnisotropicSampler);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create anisotropic sampler, hr = %d", hr);
        return false;
    }

    return true;
}

bool Renderer::CreateConstantBuffers() {
    HRESULT hr;
    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA subData = {};

    mCBMatricesData.Model = MatIdentity;
    mCBMatricesData.View = MatIdentity;
    mCBMatricesData.Projection = MatIdentity;
    mCBMatricesData.ModelView = MatIdentity;
    mCBMatricesData.ModelViewProj = MatIdentity;

    mCBSkinnedData.VScale = vec4(1.0f);
    for (auto& b : mCBSkinnedData.Bones) {
        b = MatIdentity;
    }

    // matrices CB
    desc.ByteWidth = sizeof(ConstantBufferMatrices);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    subData.pSysMem = &mCBMatricesData;
    subData.SysMemPitch = desc.ByteWidth;

    hr = mDevice->CreateBuffer(&desc, &subData, &mCBMatrices);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create matrices CB, hr = %d", hr);
        return false;
    }

    // skinned CB
    desc.ByteWidth = sizeof(ConstantBufferSkinned);
    subData.pSysMem = &mCBSkinnedData;
    subData.SysMemPitch = desc.ByteWidth;

    hr = mDevice->CreateBuffer(&desc, &subData, &mCBSkinned);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create skinned CB, hr = %d", hr);
        return false;
    }

    // terrain CB
    desc.ByteWidth = sizeof(ConstantBufferTerrain);
    subData.pSysMem = &mCBTerrainData;
    subData.SysMemPitch = desc.ByteWidth;

    hr = mDevice->CreateBuffer(&desc, &subData, &mCBTerrain);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create terrain CB, hr = %d", hr);
        return false;
    }

    // surfparams CB
    desc.ByteWidth = sizeof(ConstantBufferSurfParams);
    subData.pSysMem = &mCBSurfParams;
    subData.SysMemPitch = desc.ByteWidth;

    hr = mDevice->CreateBuffer(&desc, &subData, &mCBSurfParams);
    if (FAILED(hr)) {
        LogPrintF(LogLevel::Error, "Failed to create surf params CB, hr = %d", hr);
        return false;
    }

    return true;
}

// drawing
void Renderer::DrawFillPass() {
    ID3D11Buffer* vcbuffers[] = { mCBMatrices, mCBSkinned, mCBTerrain };
    ID3D11Buffer* pcbuffers[] = { mCBMatrices, mCBSurfParams };
    mContext->VSSetConstantBuffers(0, scast<UINT>(std::size(vcbuffers)), vcbuffers);
    mContext->PSSetConstantBuffers(0, scast<UINT>(std::size(pcbuffers)), pcbuffers);

    if (mRendererType != RendererType::Wireframe) {
        mContext->PSSetShader(mPixelShaderDefault, nullptr, 0);
    } else {
        mCBSurfParamsData.param0 = vec4(1.0f);
        D3D11_MAPPED_SUBRESOURCE mapped;
        mContext->Map(mCBSurfParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &mCBSurfParamsData, sizeof(mCBSurfParamsData));
        mContext->Unmap(mCBSurfParams, 0);

        mContext->PSSetShader(mPixelShaderSelection, nullptr, 0);
    }

    for (SceneNode* node : mLevelGeoNodes) {
        this->DrawLevelGeoNode(scast<LevelGeoNode*>(node));
    }

    if (mRendererType != RendererType::Wireframe) {
        mContext->PSSetShader(mPixelShaderDefault, nullptr, 0);
    }
    mContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    for (SceneNode* node : mModelNodes) {
        this->DrawModelNode(scast<ModelNode*>(node));
    }
}

void Renderer::DrawResolvePass() {
    static ID3D11ShaderResourceView* nullSrvs[GBuffer_NumLayers] = { nullptr };

    mContext->IASetInputLayout(nullptr);
    mContext->VSSetShader(mVertexShaderFullscreen, nullptr, 0);

    if (mRendererType == RendererType::Regular) {
        mContext->PSSetShader(mPixelShaderDeferredResolve, nullptr, 0);
    } else {
        mContext->PSSetShader(mPixelShaderDeferredDebug, nullptr, 0);

        const int renderMode = scast<int>(mRendererType) - scast<int>(RendererType::Albedo);

        mCBSurfParamsData.param0.w = scast<float>(renderMode);
        D3D11_MAPPED_SUBRESOURCE mapped;
        mContext->Map(mCBSurfParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &mCBSurfParamsData, sizeof(mCBSurfParamsData));
        mContext->Unmap(mCBSurfParams, 0);
    }

    mContext->PSSetShaderResources(0, scast<UINT>(GBuffer_NumLayers), mGBufferSRV);

    mContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    mContext->IASetIndexBuffer(nullptr, scast<DXGI_FORMAT>(0), 0);

    mContext->Draw(3, 0);

    mContext->PSSetShaderResources(0, scast<UINT>(GBuffer_NumLayers), nullSrvs);
}

void Renderer::DrawTools() {
    this->BeginDebugDraw();
    this->DebugDrawLine(vec3(0.0), vec3(1.0f, 0.0f, 0.0f), color4f(1.0f, 0.0f, 0.0f, 1.0f));    // X [Red]
    this->DebugDrawLine(vec3(0.0), vec3(0.0f, 1.0f, 0.0f), color4f(0.0f, 1.0f, 0.0f, 1.0f));    // Y [Green]
    this->DebugDrawLine(vec3(0.0), vec3(0.0f, 0.0f, 1.0f), color4f(0.0f, 0.0f, 1.0f, 1.0f));    // Z [Blue]
    this->EndDebugDraw();
}

void Renderer::DrawDebug() {
    ID3D11Buffer* gcbuffers[] = { mCBMatrices };
    mContext->GSSetConstantBuffers(0, scast<UINT>(std::size(gcbuffers)), gcbuffers);

    mContext->OMSetRenderTargets(1, &mBackBufferRTV, mBackBufferDSV);
    mContext->RSSetState(mFillRS);
    mContext->OMSetDepthStencilState(mDepthStencilState, 0);

    mContext->IASetInputLayout(mInputLayoutDebug);
    mContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mContext->VSSetShader(mVertexShaderDebugPassthrough, nullptr, 0);
    mContext->GSSetShader(mGeometryShaderDebugGenNormals, nullptr, 0);
    mContext->PSSetShader(mPixelShaderDebugLit, nullptr, 0);
    for (SceneNode* node : mDebugGeoNodes) {
        this->DrawDebugGeoNode(scast<DebugGeoNode*>(node));
    }

    mContext->GSSetShader(nullptr, nullptr, 0);
}

void Renderer::DrawModelNode(ModelNode* node) {
    if (!mFrustum.IsAABBoxIn(node->GetAABB())) {
        return;
    }

    Model* model = node->GetModel();
    const size_t lodIdx = node->GetLod();

    mCBMatricesData.Model = node->GetTransform();
    mCBMatricesData.ModelView = mCBMatricesData.View * mCBMatricesData.Model;
    mCBMatricesData.ModelViewProj = mCBMatricesData.Projection * mCBMatricesData.ModelView;
    mCBMatricesData.NormalWS = MatTranspose(MatInverse(mCBMatricesData.Model));

    D3D11_MAPPED_SUBRESOURCE mapped;
    mContext->Map(mCBMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &mCBMatricesData, sizeof(mCBMatricesData));
    mContext->Unmap(mCBMatrices, 0);

    bool isDynamic = false;
    UINT stride;
    if (model->GetType() == Model::Type::Static) {
        mContext->IASetInputLayout(mInputLayoutStatic);
        mContext->VSSetShader(mVertexShaderStatic, nullptr, 0);
        stride = sizeof(VertexStatic);
    } else if (model->GetType() == Model::Type::Skinned) {
        mContext->IASetInputLayout(mInputLayoutSkinned);
        mContext->VSSetShader(mVertexShaderSkinned, nullptr, 0);
        stride = sizeof(VertexSkinned);
        isDynamic = true;
    } else if (model->GetType() == Model::Type::Soft) {
        mContext->IASetInputLayout(mInputLayoutSoft);
        mContext->VSSetShader(mVertexShaderSoft, nullptr, 0);
        stride = sizeof(VertexSoft);
    } else {
        assert(false && "Invalid model type!");
    }

    ID3D11Buffer* vb = model->GetVertexBuffer();
    ID3D11Buffer* ib = model->GetIndexBuffer();

    const size_t numSections = model->GetNumSections(lodIdx);
    for (size_t i = 0; i < numSections; ++i) {
        const MeshSection& section = model->GetSection(i, lodIdx);
        const Surface& surface = model->GetSurface(i, lodIdx);

        if (isDynamic) {
            mCBSkinnedData.VScale.x = section.vscale;

            const Animator* anim = node->GetAnimator();
            if (anim) {
                //#TODO_SK: optimization - move vscale somewhere and copy matrices only once !!
                const Mat4Array& animResult = anim->GetAnimResult();
                for (size_t b = 0, numBones = section.bonesRemap.size(); b < numBones; ++b) {
                    mCBSkinnedData.Bones[b] = animResult[section.bonesRemap[b]];
                }
            }

            mContext->Map(mCBSkinned, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            memcpy(mapped.pData, &mCBSkinnedData, sizeof(mCBSkinnedData));
            mContext->Unmap(mCBSkinned, 0);
        }

        if (mRendererType != RendererType::Wireframe) {
            mCBSurfParamsData.param0.x = section.alphaCut;
            mContext->Map(mCBSurfParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            memcpy(mapped.pData, &mCBSurfParamsData, sizeof(mCBSurfParamsData));
            mContext->Unmap(mCBSurfParams, 0);

            ID3D11ShaderResourceView* srvs[3] = {
                surface.base->GetSRV(),
                surface.normal->GetSRV(),
                surface.bump->GetSRV()
            };

            mContext->PSSetShaderResources(0, 3, srvs);
        }

        const UINT vbOffset = section.vbOffset;
        const UINT ibOffset = section.ibOffset;

        mContext->IASetVertexBuffers(0, 1, &vb, &stride, &vbOffset);
        mContext->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, ibOffset);
        mContext->DrawIndexed(scast<UINT>(section.numIndices), 0, 0);
    }
}

void Renderer::DrawLevelGeoNode(LevelGeoNode* node) {
    LevelGeo* geo = node->GetLevelGeo();

    mCBMatricesData.Model = node->GetTransform();
    mCBMatricesData.ModelView = mCBMatricesData.View * mCBMatricesData.Model;
    mCBMatricesData.ModelViewProj = mCBMatricesData.Projection * mCBMatricesData.ModelView;
    mCBMatricesData.NormalWS = MatTranspose(MatInverse(mCBMatricesData.Model));

    D3D11_MAPPED_SUBRESOURCE mapped;
    mContext->Map(mCBMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &mCBMatricesData, sizeof(mCBMatricesData));
    mContext->Unmap(mCBMatrices, 0);

    const UINT vertexStride = sizeof(VertexLevel);
    mContext->IASetInputLayout(mInputLayoutLevelGeo);
    mContext->VSSetShader(mVertexShaderLevelGeo, nullptr, 0);

    mContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const size_t numSectors = geo->GetNumSectors();
    for (size_t i = 0; i < numSectors; ++i) {
        const LevelGeoSector& sector = geo->GetSector(i);

        for (const LevelGeoSection& section : sector.sections) {
            if (section.numIndices) {
                if (mRendererType != RendererType::Wireframe) {
                    const Surface& surface = geo->GetSurface(section.surfaceIdx);

                    ID3D11ShaderResourceView* srvs[3] = {
                        surface.base->GetSRV(),
                        surface.normal->GetSRV(),
                        surface.bump->GetSRV()
                    };

                    mContext->PSSetShaderResources(0, 3, srvs);
                }

                mContext->IASetVertexBuffers(0, 1, &sector.vertexBuffer, &vertexStride, &section.vbOffset);
                mContext->IASetIndexBuffer(sector.indexBuffer, DXGI_FORMAT_R16_UINT, section.ibOffset);
                mContext->DrawIndexed(scast<UINT>(section.numIndices), 0, 0);
            }
        }
    }

    const size_t numTerrainChunks = geo->GetTerrainNumChunks();
    if (numTerrainChunks) {
        const float hmapW = scast<float>(geo->GetTerrainHMapWidth() - 1);
        const float hmapH = scast<float>(geo->GetTerrainHMapHeight() - 1);
        const float numChunksX = scast<float>(geo->GetTerrainNumChunksX());
        const float numChunksY = scast<float>(geo->GetTerrainNumChunksY());
        const vec3& terrainMin = geo->GetTerrainMin();
        const vec3& terrainDim = geo->GetTerrainDim();

        mCBTerrainData.Terrain_Params0 = vec4(hmapW, hmapH, numChunksX, numChunksY);
        mCBTerrainData.Terrain_Params1 = vec4(terrainMin, 0.0f);
        mCBTerrainData.Terrain_Params2 = vec4(terrainDim, 0.0f);

        D3D11_MAPPED_SUBRESOURCE mapped;
        mContext->Map(mCBTerrain, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &mCBTerrainData, sizeof(mCBTerrainData));
        mContext->Unmap(mCBTerrain, 0);

        const UINT vertexStrideTerrain = sizeof(vec2);
        mContext->IASetInputLayout(mInputLayoutTerrain);

        ID3D11ShaderResourceView* vsSrvs[1] = { geo->GetTerrainHMap()->GetSRV() };
        mContext->VSSetShader(mVertexShaderTerrain, nullptr, 0);
        mContext->VSSetShaderResources(0, 1, vsSrvs);

        mContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        if (mRendererType != RendererType::Wireframe) {
            ID3D11ShaderResourceView* psSrvs[] = {
                geo->GetTerrainDiffuse()->GetSRV(),
                geo->GetTerrainNormalmap()->GetSRV(),
                geo->GetTerrainLMap()->GetSRV(),
                geo->GetTerrainMask()->GetSRV(),
                geo->GetTerrainDet(0)->GetSRV(),
                geo->GetTerrainDet(1)->GetSRV(),
                geo->GetTerrainDet(2)->GetSRV(),
                geo->GetTerrainDet(3)->GetSRV()
            };
            mContext->PSSetShader(mPixelShaderTerrain, nullptr, 0);
            mContext->PSSetShaderResources(0, scast<UINT>(std::size(psSrvs)), psSrvs);
        }

        ID3D11Buffer* vb = geo->GetTerrainVB();
        ID3D11Buffer* ib = geo->GetTerrainIB();

        mContext->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);

        const UINT vbOffset = 0;
        mContext->IASetVertexBuffers(0, 1, &vb, &vertexStrideTerrain, &vbOffset);

#if 0
        const TerrainChunk& chunk = geo->GetTerrainChunk(0);
        mContext->DrawIndexedInstanced(scast<UINT>(chunk.numIndices), scast<UINT>(numTerrainChunks), 0, 0, 0);
#else
        for (size_t i = 0; i < numTerrainChunks; ++i) {
            const TerrainChunk& chunk = geo->GetTerrainChunk(i);

            //if (mFrustum.IsAABBoxIn(chunk.bbox)) {
                mContext->DrawIndexedInstanced(scast<UINT>(chunk.numIndices), 1, 0, 0, scast<UINT>(i));
            //}
        }
#endif

        mContext->VSSetShaderResources(0, 0, nullptr);
    }
}

void Renderer::DrawDebugGeoNode(DebugGeoNode* node) {
    DebugGeo* debugGeo = node->GetDebugGeo();

    mCBMatricesData.Model = node->GetTransform();
    mCBMatricesData.ModelView = mCBMatricesData.View * mCBMatricesData.Model;
    mCBMatricesData.ModelViewProj = mCBMatricesData.Projection * mCBMatricesData.ModelView;
    mCBMatricesData.NormalWS = MatTranspose(MatInverse(mCBMatricesData.Model));

    D3D11_MAPPED_SUBRESOURCE mapped;
    mContext->Map(mCBMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &mCBMatricesData, sizeof(mCBMatricesData));
    mContext->Unmap(mCBMatrices, 0);

    const UINT stride = sizeof(VertexDebug);

    ID3D11Buffer* vb = debugGeo->GetVertexBuffer();
    ID3D11Buffer* ib = debugGeo->GetIndexBuffer();

    const size_t numSections = debugGeo->GetNumSections();
    for (size_t i = 0; i < numSections; ++i) {
        const DebugGeoSection& section = debugGeo->GetSection(i);

        const UINT vbOffset = section.vbOffset;
        const UINT ibOffset = section.ibOffset;

        mContext->IASetVertexBuffers(0, 1, &vb, &stride, &vbOffset);
        mContext->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, ibOffset);
        mContext->DrawIndexed(scast<UINT>(section.numIndices), 0, 0);
    }
}

} // namespace u4a
