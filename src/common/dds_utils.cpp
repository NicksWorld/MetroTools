#include "mycommon.h"
#include "dds_utils.h"
#include "bc7enc16.h"

#define STB_DXT_IMPLEMENTATION 1
#include "stb_dxt.h"

#define BCDEC_IMPLEMENTATION 1
#include "bcdec.h"



void DDS_DecompressBC1(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            uint8_t* dst = dest + (y * width + x) * 4;
            bcdec_bc1(src, dst, width * 4);
            src += BCDEC_BC1_BLOCK_SIZE;
        }
    }
}

void DDS_DecompressBC2(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            uint8_t* dst = dest + (y * width + x) * 4;
            bcdec_bc2(src, dst, width * 4);
            src += BCDEC_BC2_BLOCK_SIZE;
        }
    }
}

void DDS_DecompressBC3(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            uint8_t* dst = dest + (y * width + x) * 4;
            bcdec_bc3(src, dst, width * 4);
            src += BCDEC_BC3_BLOCK_SIZE;
        }
    }
}

void DDS_DecompressBC7(const void* inputBlocks, void* outPixels, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* src = rcast<const uint8_t*>(inputBlocks);
    uint8_t* dest = rcast<uint8_t*>(outPixels);

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            uint8_t* dst = dest + (y * width + x) * 4;
            bcdec_bc7(src, dst, width * 4);
            src += BCDEC_BC7_BLOCK_SIZE;
        }
    }
}

template <bool alpha>
void DDS_CompressBC_Common(const void* inputRGBA, void* outBlocks, const size_t width, const size_t height) {
    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* srcPtr = rcast<const uint8_t*>(inputRGBA);
    uint8_t* dst = rcast<uint8_t*>(outBlocks);

    uint8_t pixelsBlock[16 * 4] = { 0 };

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            const uint8_t* src = srcPtr + (y * width + x) * 4;
            for (size_t i = 0; i < 4; ++i) {
                std::memcpy(&pixelsBlock[i * 16], src, 16);
                src += (width * 4);
            }

            stb_compress_dxt_block(dst, pixelsBlock, alpha ? 1 : 0, STB_DXT_HIGHQUAL);
            dst += alpha ? 16 : 8;
        }
    }
}

void DDS_CompressBC1(const void* inputRGBA, void* outBlocks, const size_t width, const size_t height) {
    DDS_CompressBC_Common<false>(inputRGBA, outBlocks, width, height);
}

void DDS_CompressBC3(const void* inputRGBA, void* outBlocks, const size_t width, const size_t height) {
    DDS_CompressBC_Common<true>(inputRGBA, outBlocks, width, height);
}

void DDS_CompressBC7(const void* inputRGBA, void* outBlocks, const size_t width, const size_t height) {
    static bool sNeedInitBc7Comp = true;

    if (sNeedInitBc7Comp) {
        bc7enc16_compress_block_init();
        sNeedInitBc7Comp = false;
    }

    bc7enc16_compress_block_params params;
    bc7enc16_compress_block_params_init(&params);
    bc7enc16_compress_block_params_init_perceptual_weights(&params);

    const size_t sx = (width < 4) ? width : 4;
    const size_t sy = (height < 4) ? height : 4;

    const uint8_t* srcPtr = rcast<const uint8_t*>(inputRGBA);
    uint8_t* dst = rcast<uint8_t*>(outBlocks);

    uint8_t pixelsBlock[16 * 4] = { 0 };

    for (size_t y = 0; y < height; y += 4) {
        for (size_t x = 0; x < width; x += 4) {
            const uint8_t* src = srcPtr + (y * width + x) * 4;
            for (size_t i = 0; i < 4; ++i) {
                std::memcpy(&pixelsBlock[i * 16], src, 16);
                src += (width * 4);
            }

            bc7enc16_compress_block(dst, pixelsBlock, &params);
            dst += 16;
        }
    }
}

size_t DDS_GetCompressedSizeBC1(const size_t width, const size_t height, const size_t numMips) {
    size_t w = width;
    size_t h = height;
    size_t result = 0;

    for (size_t i = 0; i < numMips; ++i) {
        w = std::max<size_t>(4, w);
        h = std::max<size_t>(4, h);

        result += BCDEC_BC1_COMPRESSED_SIZE(w, h);

        w /= 2;
        h /= 2;
    }

    return result;
}

size_t DDS_GetCompressedSizeBC7(const size_t width, const size_t height, const size_t numMips) {
    size_t w = width;
    size_t h = height;
    size_t result = 0;

    for (size_t i = 0; i < numMips; ++i) {
        w = std::max<size_t>(4, w);
        h = std::max<size_t>(4, h);

        result += BCDEC_BC7_COMPRESSED_SIZE(w, h);

        w /= 2;
        h /= 2;
    }

    return result;
}


void DDS_MakeDX10Headers(DDSURFACEDESC2& desc, DDS_HEADER_DXT10& dx10hdr, const size_t w, const size_t h, const size_t numMips, const bool isCube) {
    memset(&desc, 0, sizeof(desc));
    memset(&dx10hdr, 0, sizeof(dx10hdr));

    desc.dwSize = sizeof(DDSURFACEDESC2);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.dwWidth = scast<uint32_t>(w);
    desc.dwHeight = scast<uint32_t>(h);
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    desc.ddpfPixelFormat.dwFourCC = PIXEL_FMT_FOURCC('D', 'X', '1', '0');
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

    if (numMips > 1) {
        desc.dwFlags |= DDSD_MIPMAPCOUNT;
        desc.ddsCaps.dwCaps |= DDSCAPS_COMPLEX;
        desc.dwMipMapCount = scast<uint32_t>(numMips);
    }

    if (isCube) {
        desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP |
                               DDSCAPS2_CUBEMAP_POSITIVEX |
                               DDSCAPS2_CUBEMAP_NEGATIVEX |
                               DDSCAPS2_CUBEMAP_POSITIVEY |
                               DDSCAPS2_CUBEMAP_NEGATIVEY |
                               DDSCAPS2_CUBEMAP_POSITIVEZ |
                               DDSCAPS2_CUBEMAP_NEGATIVEZ;
    }

    dx10hdr.dxgiFormat = isCube ? DXGI_FORMAT_BC6H_UF16 : DXGI_FORMAT_BC7_UNORM;
    dx10hdr.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
    dx10hdr.miscFlag = isCube ? 4 : 0;
    dx10hdr.arraySize = 1;
    dx10hdr.miscFlags2 = 0;
}

void DDS_MakeDX9Header(DDSURFACEDESC2& desc, const size_t w, const size_t h, const size_t numMips) {
    memset(&desc, 0, sizeof(desc));

    desc.dwSize = sizeof(DDSURFACEDESC2);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.dwWidth = scast<uint32_t>(w);
    desc.dwHeight = scast<uint32_t>(h);
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    desc.ddpfPixelFormat.dwFourCC = PIXEL_FMT_DXT5;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

    if (numMips) {
        desc.dwFlags |= DDSD_MIPMAPCOUNT;
        desc.ddsCaps.dwCaps |= DDSCAPS_MIPMAP;
        desc.dwMipMapCount = scast<uint32_t>(numMips);
    }
}
