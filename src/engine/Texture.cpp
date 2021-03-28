#include "Texture.h"
#include "Renderer.h"

static size_t NumMipsFromResolution(const size_t resolution) {
    size_t result = 0;

    unsigned long index;
    if (_BitScanReverse64(&index, resolution)) {
        result = static_cast<size_t>(index) + 1;
    }

    return result;
}

namespace u4a {

static DXGI_FORMAT sPixelFormatToTextureFormat[] = {
    DXGI_FORMAT_BC1_TYPELESS,
    DXGI_FORMAT_BC3_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_BC6H_TYPELESS,
    DXGI_FORMAT_BC7_TYPELESS,
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R32G8X24_TYPELESS,          // ???
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R16G16_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R16_TYPELESS,
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_R32G32B32A32_TYPELESS,
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_B8G8R8A8_TYPELESS
};


static DXGI_FORMAT sPixelFormatToSRVFormatSRGB[] = {
    DXGI_FORMAT_BC1_UNORM_SRGB,
    DXGI_FORMAT_BC3_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_BC6H_UF16,
    DXGI_FORMAT_BC7_UNORM_SRGB,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,       // ???
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UNORM,          // we don't have RGB10_SNORM_A2_UNORM on PC DirectX
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
};

static DXGI_FORMAT sPixelFormatToSRVFormatLinear[] = {
    DXGI_FORMAT_BC1_UNORM,
    DXGI_FORMAT_BC3_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_BC6H_UF16,
    DXGI_FORMAT_BC7_UNORM,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,       // ???
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UNORM,          // we don't have RGB10_SNORM_A2_UNORM on PC DirectX
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_B8G8R8A8_UNORM
};

static bool IsCompressedFormat(const Texture::Format fmt) {
    bool result = false;

    switch (fmt) {
        case Texture::Format::BC1:
        case Texture::Format::BC3:
        case Texture::Format::BC6H:
        case Texture::Format::BC7:
            result = true;
            break;

        default:
            result = false;
    }

    return result;
}

static size_t GetBCBlockSize(const Texture::Format fmt) {
    return (fmt == Texture::Format::BC1) ? 8 : 16;
}

static size_t GetBytesPerPixel(const Texture::Format fmt) {
    size_t result = 0;

    switch (fmt) {
        case Texture::Format::RGBA8_UNORM:
        case Texture::Format::RGBA8_SNORM:
        case Texture::Format::DEPTH_32F:
        case Texture::Format::R32_F:
        case Texture::Format::RG16_F:
        case Texture::Format::RGB10_UNORM_A2_UNORM:
        case Texture::Format::RGB10_SNORM_A2_UNORM:
        case Texture::Format::R11G11B10_F:
        case Texture::Format::R32_U:
        case Texture::Format::BGRA8_UNORM:
            result = 4;
            break;

        case Texture::Format::RG8_UNORM:
        case Texture::Format::RG8_SNORM:
        case Texture::Format::R16_UNORM:
            result = 2;
            break;

        case Texture::Format::RGBA16_F:
        case Texture::Format::RGBA16_U:
            result = 8;
            break;

        case Texture::Format::R8_UNORM:
        case Texture::Format::R8_U:
            result = 1;
            break;

        case Texture::Format::RGBA32_F:
            result = 16;
            break;
    }

    return result;
}

// returns line pitch
static size_t CalculatePitches(const Texture::Format fmt, const size_t width, const size_t height, size_t& slicePitch) {
    size_t linePitch = 0;

    if (IsCompressedFormat(fmt)) {
        const size_t blockSize = GetBCBlockSize(fmt);
        const size_t numBlocksW = (width + 3) / 4;
        const size_t numBlocksH = (height + 3) / 4;

        linePitch = numBlocksW * blockSize;
        slicePitch = linePitch * numBlocksH;
    } else {
        const size_t bpp = GetBytesPerPixel(fmt);
        linePitch = width * bpp;
        slicePitch = linePitch * height;
    }

    return linePitch;
}


Texture::Texture()
    : mFormat(Texture::Format::RGBA8_UNORM)
    , mWidth(0)
    , mHeight(0)
    , mNumMips(0)
    , mIsCubemap(false)
    , mIsLinear(true)
    , mGenMips(false)
    , mTexture(nullptr)
    , mSRV(nullptr)
{
}
Texture::~Texture() {
    this->Shutdown();
}

bool Texture::CreateImmutable2D(const Format fmt, const size_t width, const size_t height, const size_t numMips, const void* data, const size_t flags) {
    bool result = false;

    mFormat = fmt;
    mWidth = width;
    mHeight = height;
    mNumMips = numMips;
    mIsCubemap = false;
    mIsLinear = !TestBit(flags, Flag_SRGB);

    if (TestBit(flags, Flag_GenMips)) {
        mNumMips = NumMipsFromResolution(std::max(width, height));
        mGenMips = true;
    }

    this->CollectMipPointers(data);

    if (this->CreateTexturePrivate()) {
        result = this->CreateSRVPrivate();

        if (result && mGenMips) {
            ID3D11DeviceContext* context = Renderer::Get().GetContext();
            context->GenerateMips(mSRV);
        }
    }

    mTempMips.clear();

    return result;
}

bool Texture::CreateStreamable2D(const Format fmt, const size_t width, const size_t height, const size_t numMips, const size_t flags) {
    bool result = true;

    mFormat = fmt;
    mWidth = width;
    mHeight = height;
    mNumMips = numMips;
    mIsCubemap = false;
    mIsLinear = !TestBit(flags, Flag_SRGB);

    if (TestBit(flags, Flag_GenMips)) {
        mNumMips = NumMipsFromResolution(std::max(width, height));
        mGenMips = true;
    }

    mTempMips.resize(mNumMips);

    return result;
}

bool Texture::FinishStreamable2D() {
    bool result = false;

    if (this->CreateTexturePrivate()) {
        result = this->CreateSRVPrivate();

        if (result && mGenMips) {
            ID3D11DeviceContext* context = Renderer::Get().GetContext();
            context->GenerateMips(mSRV);
        }
    }

    mTempMips.clear();

    return result;
}

bool Texture::CreateImmutableCube(const Format fmt, const size_t width, const size_t height, const size_t numMips, const void* data, const size_t flags) {
    bool result = false;

    mFormat = fmt;
    mWidth = width;
    mHeight = height;
    mNumMips = numMips;
    mIsCubemap = true;
    mIsLinear = !TestBit(flags, Flag_SRGB);

    if (TestBit(flags, Flag_GenMips)) {
        mNumMips = NumMipsFromResolution(std::max(width, height));
        mGenMips = true;
    }

    this->CollectMipPointers(data);

    if (this->CreateTexturePrivate()) {
        result = this->CreateSRVPrivate();

        if (result && mGenMips) {
            ID3D11DeviceContext* context = Renderer::Get().GetContext();
            context->GenerateMips(mSRV);
        }
    }

    mTempMips.clear();

    return result;
}

void Texture::Shutdown() {
    MySafeRelease(mSRV);
    MySafeRelease(mTexture);
}

void Texture::UploadMips(const size_t startMip, const size_t numMips, const size_t slice, const void* data) {
    const size_t minMipSize = IsCompressedFormat(mFormat) ? 4 : 1;

    size_t mipWidth = (std::max)(mWidth >> startMip, minMipSize);
    size_t mipHeight = (std::max)(mHeight >> startMip, minMipSize);

    ID3D11DeviceContext* context = Renderer::Get().GetContext();

    const uint8_t* dataPtr = rcast<const uint8_t*>(data);
    for (size_t i = 0; i < numMips; ++i) {
        const UINT subresource = D3D11CalcSubresource(scast<UINT>(startMip + i), scast<UINT>(slice), scast<UINT>(mNumMips));

        mTempMips[subresource] = dataPtr;

        size_t slicePitch = 0;
        CalculatePitches(mFormat, mipWidth, mipHeight, slicePitch);

        dataPtr += slicePitch;

        mipWidth = (std::max)(mipWidth / 2, minMipSize);
        mipHeight = (std::max)(mipHeight / 2, minMipSize);
    }
}

Texture::Format Texture::GetFormat() const {
    return mFormat;
}

size_t Texture::GetWidth() const {
    return mWidth;
}

size_t Texture::GetHeight() const {
    return mHeight;
}

size_t Texture::GetNumMips() const {
    return mNumMips;
}

bool Texture::IsCubemap() const {
    return mIsCubemap;
}

ID3D11ShaderResourceView* Texture::GetSRV() const {
    return mSRV;
}


void Texture::CollectMipPointers(const void* data) {
    const size_t arraySize = mIsCubemap ? 6 : 1;
    const size_t numDataMips = mGenMips ? 1 : mNumMips;

    mTempMips.reserve(arraySize * numDataMips);

    const uint8_t* dataPtr = rcast<const uint8_t*>(data);

    const size_t minMipSize = IsCompressedFormat(mFormat) ? 4 : 1;

    size_t counter = 0;
    size_t mipWidth = mWidth;
    size_t mipHeight = mHeight;
    for (size_t j = 0; j < arraySize; ++j) {
        for (size_t i = 0; i < numDataMips; ++i) {
            size_t slicePitch = 0;
            const size_t linePitch = CalculatePitches(mFormat, mipWidth, mipHeight, slicePitch);

            mTempMips.push_back(dataPtr);

            dataPtr += slicePitch;

            mipWidth = (std::max)(mipWidth / 2, minMipSize);
            mipHeight = (std::max)(mipHeight / 2, minMipSize);

            ++counter;
        }
    }
}

bool Texture::CreateTexturePrivate() {
    bool result = false;

    D3D11_TEXTURE2D_DESC desc = {};

    const DXGI_FORMAT textureFormat = sPixelFormatToTextureFormat[scast<size_t>(mFormat)];

    desc.Width = scast<UINT>(mWidth);
    desc.Height = scast<UINT>(mHeight);
    desc.MipLevels = scast<UINT>(mGenMips ? 0 : mNumMips);
    desc.ArraySize = mIsCubemap ? 6 : 1;
    desc.Format = textureFormat;
    desc.SampleDesc.Count = 1;
    desc.Usage = mGenMips ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (mIsCubemap) {
        desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    }
    if (mGenMips) {
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    MyArray<D3D11_SUBRESOURCE_DATA> subDesc;

    if (!mTempMips.empty() && !mGenMips) {
        subDesc.resize(desc.ArraySize * mNumMips);

        const size_t minMipSize = IsCompressedFormat(mFormat) ? 4 : 1;

        size_t counter = 0;
        size_t mipWidth = mWidth;
        size_t mipHeight = mHeight;

        size_t ptrIdx = 0;
        for (size_t j = 0; j < desc.ArraySize; ++j) {
            for (size_t i = 0; i < mNumMips; ++i, ++ptrIdx) {
                size_t slicePitch = 0;
                const size_t linePitch = CalculatePitches(mFormat, mipWidth, mipHeight, slicePitch);

                subDesc[counter].pSysMem = mTempMips[ptrIdx];
                subDesc[counter].SysMemPitch = scast<UINT>(linePitch);
                subDesc[counter].SysMemSlicePitch = scast<UINT>(slicePitch);

                mipWidth = (std::max)(mipWidth / 2, minMipSize);
                mipHeight = (std::max)(mipHeight / 2, minMipSize);

                ++counter;
            }
        }
    }

    ID3D11Device* device = Renderer::Get().GetDevice();

    ID3D11Texture2D* tex2D = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, subDesc.empty() ? nullptr : subDesc.data(), &tex2D);
    if (SUCCEEDED(hr)) {
        if (mGenMips) {
            ID3D11DeviceContext* context = Renderer::Get().GetContext();
            size_t slicePitch = 0;
            const size_t linePitch = CalculatePitches(mFormat, mWidth, mHeight, slicePitch);
            context->UpdateSubresource(tex2D, 0, nullptr, mTempMips[0], static_cast<UINT>(linePitch), static_cast<UINT>(slicePitch));
        }

        mTexture = tex2D;
        result = true;
    }
    return result;
}

bool Texture::CreateSRVPrivate() {
    bool result = false;

    const DXGI_FORMAT srvFormat = mIsLinear ? sPixelFormatToSRVFormatLinear[scast<size_t>(mFormat)] : sPixelFormatToSRVFormatSRGB[scast<size_t>(mFormat)];

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = srvFormat;
    srvDesc.ViewDimension = mIsCubemap ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = scast<UINT>(mNumMips);
    srvDesc.Texture2D.MostDetailedMip = 0;

    ID3D11Device* device = Renderer::Get().GetDevice();

    HRESULT hr = device->CreateShaderResourceView(mTexture, &srvDesc, &mSRV);
    if (SUCCEEDED(hr)) {
        result = true;
    }

    return result;
}

} // namespace u4a

