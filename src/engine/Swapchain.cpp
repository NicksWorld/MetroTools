#include "Swapchain.h"

#include <dxgi.h>
#include <d3d11.h>

#include "log.h"

namespace u4a {

Swapchain::Swapchain()
    : mHasStencil(false)
    , mWidth(0)
    , mHeight(0)
    , mSwapchain(nullptr)
    , mBackBuffer(nullptr)
    , mDepthBuffer(nullptr)
    , mBackBufferRTV(nullptr)
    , mDepthBufferDSV(nullptr)
    , mBackBufferSRV(nullptr)
    , mDepthBufferSRV(nullptr)
{
}
Swapchain::~Swapchain() {
    this->Shutdown();
}

bool Swapchain::Initialize(ID3D11Device* device, void* hwnd) {
    bool result = false;

    mHasStencil = true;

    IDXGIFactory* factory = nullptr;
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
    if (SUCCEEDED(hr)) {
        RECT rc;
        ::GetClientRect(scast<HWND>(hwnd), &rc);

        const UINT wndWidth = scast<UINT>(rc.right - rc.left);
        const UINT wndHeight = scast<UINT>(rc.bottom - rc.top);

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = wndWidth;
        swapChainDesc.BufferDesc.Height = wndHeight;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = scast<HWND>(hwnd);
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = true;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        hr = factory->CreateSwapChain(device, &swapChainDesc, &mSwapchain);
        if (SUCCEEDED(hr)) {
            mWidth = wndWidth;
            mHeight = wndHeight;

            result = this->CreateDeviceObjects();
        } else {
            LogPrintF(LogLevel::Error, "Failed to create swapchain, hr = %d", hr);
        }
    } else {
        LogPrintF(LogLevel::Error, "Failed to create DXGI Factory, hr = %d", hr);
    }

    MySafeRelease(factory);

    return result;
}

void Swapchain::Shutdown() {
    this->DestroyDeviceObjects();

    MySafeRelease(mSwapchain);
}

void Swapchain::Resize(const size_t newWidth, const size_t newHeight) {
    this->DestroyDeviceObjects();

    HRESULT hr = mSwapchain->ResizeBuffers(1, scast<UINT>(newWidth), scast<UINT>(newHeight), DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr)) {
        mWidth = newWidth;
        mHeight = newHeight;
        this->CreateDeviceObjects();
    }
}

size_t Swapchain::GetWidth() const {
    return mWidth;
}

size_t Swapchain::GetHeight() const {
    return mHeight;
}

ID3D11Texture2D* Swapchain::GetBackbuffer() const {
    return mBackBuffer;
}

ID3D11RenderTargetView* Swapchain::GetRTV() const {
    return mBackBufferRTV;
}

ID3D11DepthStencilView* Swapchain::GetDSV() const {
    return mDepthBufferDSV;
}

ID3D11ShaderResourceView* Swapchain::GetFrameSRV() const {
    return mBackBufferSRV;
}

ID3D11ShaderResourceView* Swapchain::GetDepthSRV() const {
    return mDepthBufferSRV;
}

void Swapchain::Present() {
    mSwapchain->Present(0, 0);
}


bool Swapchain::CreateDeviceObjects() {
    HRESULT hr = mSwapchain->GetBuffer(0, IID_PPV_ARGS(&mBackBuffer));
    if (FAILED(hr)) {
        return false;
    }

    ID3D11Device* device = nullptr;
    mSwapchain->GetDevice(IID_PPV_ARGS(&device));

    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = scast<UINT>(mWidth);
    depthBufferDesc.Height = scast<UINT>(mHeight);
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = mHasStencil ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    hr = device->CreateTexture2D(&depthBufferDesc, nullptr, &mDepthBuffer);
    if (FAILED(hr)) {
        return false;
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = device->CreateRenderTargetView(mBackBuffer, nullptr, &mBackBufferRTV);
    if (FAILED(hr)) {
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = mHasStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = device->CreateDepthStencilView(mDepthBuffer, &depthStencilViewDesc, &mDepthBufferDSV);
    if (FAILED(hr)) {
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    //hr = device->CreateShaderResourceView(mBackBuffer, &srvDesc, &mBackBufferSRV);
    //if (FAILED(hr)) {
    //    return false;
    //}

    srvDesc.Format = mHasStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
    hr = device->CreateShaderResourceView(mDepthBuffer, &srvDesc, &mDepthBufferSRV);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void Swapchain::DestroyDeviceObjects() {
    MySafeRelease(mDepthBufferSRV);
    MySafeRelease(mBackBufferSRV);
    MySafeRelease(mDepthBufferDSV);
    MySafeRelease(mBackBufferRTV);
    MySafeRelease(mDepthBuffer);
    MySafeRelease(mBackBuffer);
}

} // namespace u4a
