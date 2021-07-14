#pragma once
#include "mycommon.h"

struct MetroCompression {
    enum {
        Type_Unknown    = 0,
        Type_LZ4        = 1
    };

    // Original 2033 and Last Light
    // QuickLZ
    static size_t DecompressStreamLegacy(const void* compressedData, const size_t compressedSize, void* uncompressedData, const size_t uncompressedSize);
    static size_t CompressStreamLegacy(const void* data, const size_t dataLength, BytesArray& compressed);
    //


    // Redux / Arktika.1 / Exodus
    // simply LZ4
    static size_t DecompressStream(const void* compressedData, const size_t compressedSize, void* uncompressedData, const size_t uncompressedSize);
    static size_t DecompressBlob(const void* compressedData, const size_t compressedSize, void* uncompressedData, const size_t uncompressedSize);

    static size_t CompressStream(const void* data, const size_t dataLength, BytesArray& compressed);
    static size_t CompressBlob(const void* data, const size_t dataLength, BytesArray& compressed);
    //
};
