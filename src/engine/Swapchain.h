#pragma once
#include "mycommon.h"

// fwd declaration of DirectX interfaces
struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;

namespace u4a {

class Swapchain {
public:
    Swapchain();
    ~Swapchain();

    bool                        Initialize(ID3D11Device* device, void* hwnd);
    void                        Shutdown();
    void                        Resize(const size_t newWidth, const size_t newHeight);

    size_t                      GetWidth() const;
    size_t                      GetHeight() const;

    ID3D11Texture2D*            GetBackbuffer() const;

    ID3D11RenderTargetView*     GetRTV() const;
    ID3D11DepthStencilView*     GetDSV() const;
    ID3D11ShaderResourceView*   GetFrameSRV() const;
    ID3D11ShaderResourceView*   GetDepthSRV() const;

    void                        Present();

private:
    bool                        CreateDeviceObjects();
    void                        DestroyDeviceObjects();

private:
    bool                        mHasStencil;
    size_t                      mWidth;
    size_t                      mHeight;
    IDXGISwapChain*             mSwapchain;
    ID3D11Texture2D*            mBackBuffer;
    ID3D11Texture2D*            mDepthBuffer;
    ID3D11RenderTargetView*     mBackBufferRTV;
    ID3D11DepthStencilView*     mDepthBufferDSV;
    ID3D11ShaderResourceView*   mBackBufferSRV;
    ID3D11ShaderResourceView*   mDepthBufferSRV;

};

} // namespace u4a
