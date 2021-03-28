#pragma once
#include "mycommon.h"

struct ID3D11Resource;
struct ID3D11ShaderResourceView;

namespace u4a {

class Texture {
public:
    enum class Format : size_t {        // this is basically same enum as in MetroTexture
        BC1                     = 0,
        BC3                     = 1,
        RGBA8_UNORM             = 2,
        RGBA8_SNORM             = 3,
        BC6H                    = 4,
        BC7                     = 5,
        RG8_UNORM               = 6,
        RG8_SNORM               = 7,
        DEPTH_32F_S8            = 8,
        DEPTH_32F               = 9,
        R32_F                   = 10,
        RGBA16_F                = 11,
        RG16_F                  = 12,
        RGBA16_U                = 13,
        R8_UNORM                = 14,
        R8_U                    = 15,
        RGB10_UNORM_A2_UNORM    = 16,
        RGB10_SNORM_A2_UNORM    = 17,   //#NOTE_SK: this format is unavailable on PC for DirectX, tho available on Vulkan and XBox
        R11G11B10_F             = 18,
        R16_UNORM               = 19,
        R32_U                   = 20,
        RGBA32_F                = 21,
        PPS                     = 22,   //#NOTE_SK: have no clue wtf is this
        BGRA8_UNORM             = 23
    };

    static const size_t Flag_SRGB       = 1;
    static const size_t Flag_GenMips    = 2;

public:
    Texture();
    ~Texture();

    bool                        CreateImmutable2D(const Format fmt, const size_t width, const size_t height, const size_t numMips, const void* data, const size_t flags);
    bool                        CreateStreamable2D(const Format fmt, const size_t width, const size_t height, const size_t numMips, const size_t flags);
    bool                        FinishStreamable2D();
    bool                        CreateImmutableCube(const Format fmt, const size_t width, const size_t height, const size_t numMips, const void* data, const size_t flags);
    void                        Shutdown();

    void                        UploadMips(const size_t startMip, const size_t numMips, const size_t slice, const void* data);

    Format                      GetFormat() const;
    size_t                      GetWidth() const;
    size_t                      GetHeight() const;
    size_t                      GetNumMips() const;
    bool                        IsCubemap() const;
    ID3D11ShaderResourceView*   GetSRV() const;

private:
    void                        CollectMipPointers(const void* data);
    bool                        CreateTexturePrivate();
    bool                        CreateSRVPrivate();

private:
    Format                      mFormat;
    size_t                      mWidth;
    size_t                      mHeight;
    size_t                      mNumMips;
    bool                        mIsCubemap;
    bool                        mIsLinear;
    bool                        mGenMips;
    ID3D11Resource*             mTexture;
    ID3D11ShaderResourceView*   mSRV;

    //
    MyArray<const void*>        mTempMips;
};

} // namespace u4a
