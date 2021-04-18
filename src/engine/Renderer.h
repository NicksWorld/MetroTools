#pragma once
#include <d3d11.h>

#include "mycommon.h"
#include "mymath.h"

namespace u4a {

// Forward declarations
class Scene;
class SceneNode;
class Swapchain;
class ModelNode;
class LevelGeoNode;

#include "RendererTypes.inl"

class Renderer {
    enum GBufferIdx : size_t {
        GBuffer_Base = 0,
        GBuffer_Normals,

        GBuffer_NumLayers
    };

    static vec4 kClearColor[GBuffer_NumLayers];

    Renderer();
    ~Renderer();

public:
    enum InitFlags : size_t {
        IF_None = 0
    };

    IMPL_SINGLETON(Renderer)

public:
    bool                        CreateDevice(const size_t flags = Renderer::IF_None);
    bool                        Initialize();
    void                        Shutdown();

    ID3D11Device*               GetDevice();
    ID3D11DeviceContext*        GetContext();

    void                        StartFrame(Swapchain& swapchain);
    void                        DrawScene(Scene& scene);

    SceneNode*                  PickObject(Swapchain& swapchain, Scene& scene, const vec2& screenPoint);

    void                        BeginDebugDraw();
    void                        DebugDrawBBox(const AABBox& bbox, const color4f& color);
    void                        DebugDrawRing(const vec3& origin, const vec3& majorAxis, const vec3& minorAxis, const color4f& color);
    void                        DebugDrawBSphere(const BSphere& bsphere, const color4f& color);
    void                        DebugDrawTetrahedron(const vec3& a, const vec3& b, const float r, const color4f& color);
    void                        EndDebugDraw();

private:
    void                        EnsureDebugVertices(const size_t required);
    void                        FlushDebugVertices();

    void                        CheckGBuffer(const Swapchain& swapchain);
    void                        CheckSelectionRT(const Swapchain& swapchain);

    bool                        CreateShaders();
    bool                        CreateVertexInputLayouts();
    bool                        CreateSamplers();
    bool                        CreateConstantBuffers();

    // drawing
    void                        DrawFillPass();
    void                        DrawResolvePass();
    void                        DrawTools();
    void                        DrawDebug();

    void                        DrawModelNode(ModelNode* node);
    void                        DrawLevelGeoNode(LevelGeoNode* node);

private:
    ID3D11Device*               mDevice;
    ID3D11DeviceContext*        mContext;

    // Swapchain temp stuff
    ID3D11RenderTargetView*     mBackBufferRTV;
    ID3D11DepthStencilView*     mBackBufferDSV;

    // GBuffer
    size_t                      mGBufferWidth;
    size_t                      mGBufferHeight;
    ID3D11Texture2D*            mGBufferRT[GBuffer_NumLayers];
    ID3D11RenderTargetView*     mGBufferRTV[GBuffer_NumLayers];
    ID3D11ShaderResourceView*   mGBufferSRV[GBuffer_NumLayers];
    // Selection target
    ID3D11Texture2D*            mSelectionRT;
    ID3D11RenderTargetView*     mSelectionRTV;
    ID3D11Texture2D*            mSelectionReadback;
    size_t                      mSelectionWidth;
    size_t                      mSelectionHeight;

    // Rasterizer states
    ID3D11RasterizerState*      mFillRS;
    ID3D11RasterizerState*      mWireframeRS;

    // Depth-stencil states
    ID3D11DepthStencilState*    mDepthStencilState;
    ID3D11DepthStencilState*    mDepthStencilStateResolve;
    ID3D11DepthStencilState*    mDepthStencilStateDebug;

    // Built-in shaders
    // vs
    ID3D11VertexShader*         mVertexShaderStatic;
    ID3D11VertexShader*         mVertexShaderSkinned;
    ID3D11VertexShader*         mVertexShaderLevelGeo;
    ID3D11VertexShader*         mVertexShaderSoft;
    ID3D11VertexShader*         mVertexShaderTerrain;
    ID3D11VertexShader*         mVertexShaderFullscreen;
    ID3D11VertexShader*         mVertexShaderDebug;
    // ps
    ID3D11PixelShader*          mPixelShaderDefault;
    ID3D11PixelShader*          mPixelShaderTerrain;
    ID3D11PixelShader*          mPixelShaderDeferredResolve;
    ID3D11PixelShader*          mPixelShaderDebug;
    ID3D11PixelShader*          mPixelShaderSelection;

    // Vertex input layouts
    ID3D11InputLayout*          mInputLayoutStatic;
    ID3D11InputLayout*          mInputLayoutSkinned;
    ID3D11InputLayout*          mInputLayoutLevelGeo;
    ID3D11InputLayout*          mInputLayoutSoft;
    ID3D11InputLayout*          mInputLayoutTerrain;
    ID3D11InputLayout*          mInputLayoutDebug;

    // Samplers states
    ID3D11SamplerState*         mPointSampler;
    ID3D11SamplerState*         mTrilinearSampler;
    ID3D11SamplerState*         mAnisotropicSampler;

    // Constant buffers
    ConstantBufferMatrices      mCBMatricesData;
    ConstantBufferSkinned       mCBSkinnedData;
    ConstantBufferTerrain       mCBTerrainData;
    ConstantBufferSurfParams    mCBSurfParamsData;
    ID3D11Buffer*               mCBMatrices;
    ID3D11Buffer*               mCBSkinned;
    ID3D11Buffer*               mCBTerrain;
    ID3D11Buffer*               mCBSurfParams;

    MyArray<SceneNode*>         mModelNodes;
    MyArray<SceneNode*>         mLevelGeoNodes;

    Frustum                     mFrustum;
    bool                        mUpdateFrustum;

    // Debug draw
    ID3D11Buffer*               mDebugVertexBuffer;
    void*                       mDebugVerticesPtr;
    size_t                      mDebugVerticesCount;
};

} // namespace u4a
